/*
  ==============================================================================

	guitarSeparator.cpp
	Created: 24 Sep 2025 1:27:21am
	Author:  eigil

  ==============================================================================
*/

#include <JuceHeader.h>
#include "guitarSeparator.h"
#include <cmath>

#if defined(AA_ENABLE_ONNXRUNTIME) && AA_ENABLE_ONNXRUNTIME && __has_include(<onnxruntime_cxx_api.h>)
 #include <onnxruntime_cxx_api.h>
 #define AUDIO_ANALYZER_HAS_ONNXRUNTIME 1
#else
 #define AUDIO_ANALYZER_HAS_ONNXRUNTIME 0
#endif

namespace
{
constexpr int defaultModelSampleRate = 44100;
constexpr int defaultModelChannels = 2;
constexpr int defaultModelWindowFrames = 8192;
constexpr int separatorIoChannels = 2;
constexpr int defaultInputCapacityFrames = 65536;
constexpr int defaultOutputCapacityFrames = 65536;
constexpr float fallbackHighPassHz = 70.0f;
constexpr float fallbackLowPassHz = 6000.0f;
constexpr float fallbackGateThreshold = 0.01f;
constexpr float fallbackResidualGain = 0.35f;
constexpr auto modelUrlEnvVar = "AUDIO_ANALYZER_HTDEMUCS_MODEL_URL";
constexpr auto manifestUrlEnvVar = "AUDIO_ANALYZER_HTDEMUCS_MANIFEST_URL";

void resampleInterleaved(const std::vector<float>& input,
	int inputFrames,
	std::vector<float>& output,
	int outputFrames,
	int channels)
{
	output.assign(static_cast<size_t>(outputFrames * channels), 0.0f);
	if (inputFrames <= 0 || outputFrames <= 0)
		return;
	if (inputFrames == outputFrames)
	{
		output = input;
		return;
	}

	constexpr int filterRadius = 8;
	const auto cutoff = 0.97 * juce::jmin(1.0, static_cast<double>(outputFrames) / inputFrames);
	for (int frame = 0; frame < outputFrames; ++frame)
	{
		const auto position = (static_cast<double>(frame) + 0.5) * inputFrames / outputFrames - 0.5;
		const auto firstFrame = static_cast<int>(std::floor(position)) - filterRadius + 1;
		std::array<double, 16> weights{};
		double weightSum = 0.0;

		for (int tap = 0; tap < static_cast<int>(weights.size()); ++tap)
		{
			const auto sourceFrame = firstFrame + tap;
			const auto distance = position - sourceFrame;
			const auto windowPosition = distance / filterRadius;
			if (sourceFrame < 0 || sourceFrame >= inputFrames || std::abs(windowPosition) >= 1.0)
				continue;

			const auto window = 0.5 + 0.5 * std::cos(juce::MathConstants<double>::pi * windowPosition);
			const auto scaledDistance = distance * cutoff;
			const auto sinc = std::abs(scaledDistance) < 1.0e-12
				? cutoff
				: std::sin(juce::MathConstants<double>::pi * scaledDistance)
					/ (juce::MathConstants<double>::pi * distance);
			weights[static_cast<size_t>(tap)] = sinc * window;
			weightSum += weights[static_cast<size_t>(tap)];
		}

		for (int channel = 0; channel < channels; ++channel)
		{
			double value = 0.0;
			for (int tap = 0; tap < static_cast<int>(weights.size()); ++tap)
			{
				const auto sourceFrame = firstFrame + tap;
				if (sourceFrame >= 0 && sourceFrame < inputFrames)
					value += input[static_cast<size_t>(sourceFrame * channels + channel)] * weights[static_cast<size_t>(tap)];
			}
			output[static_cast<size_t>(frame * channels + channel)]
				= weightSum != 0.0 ? static_cast<float>(value / weightSum) : 0.0f;
		}
	}
}
}

struct guitarSeparator::HtDemucsOnnxBackend
{
	bool initialise(const juce::File& modelFile,
					const ModelManifest& manifest,
					juce::String& error)
	{
#if AUDIO_ANALYZER_HAS_ONNXRUNTIME
		try
		{
			env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "AudioAnalyzerHTDemucs");

			Ort::SessionOptions sessionOptions;
			sessionOptions.SetIntraOpNumThreads(1);
			sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

		   #if JUCE_WINDOWS
			session = std::make_unique<Ort::Session>(*env,
													 modelFile.getFullPathName().toWideCharPointer(),
													 sessionOptions);
		   #else
			const auto modelPath = modelFile.getFullPathName().toStdString();
			session = std::make_unique<Ort::Session>(*env,
													 modelPath.c_str(),
													 sessionOptions);
		   #endif

			inputShape = { 1, static_cast<int64_t>(manifest.channels), static_cast<int64_t>(manifest.windowFrames) };
			inputName = session->GetInputNameAllocated(0, allocator).get();
			outputName = session->GetOutputNameAllocated(0, allocator).get();
			return true;
		}
		catch (const Ort::Exception& exception)
		{
			error = "Failed to initialise ONNX Runtime session: " + juce::String(exception.what());
			session.reset();
			env.reset();
			return false;
		}
#else
		juce::ignoreUnused(modelFile, manifest);
		error = "ONNX Runtime support is not compiled into this build.";
		return false;
#endif
	}

	bool processWindow(const std::vector<float>& interleavedInput,
					   std::vector<float>& interleavedOutput,
					   const ModelManifest& manifest,
					   juce::String& error)
	{
#if AUDIO_ANALYZER_HAS_ONNXRUNTIME
		if (session == nullptr)
		{
			error = "HTDemucs session is not available.";
			return false;
		}

		try
		{
			constexpr int ioChannels = separatorIoChannels;
			const auto frames = manifest.windowFrames;
			const auto channels = manifest.channels;

			channelMajorInput.assign(static_cast<size_t>(channels * frames), 0.0f);
			if (interleavedInput.size() < static_cast<size_t>(frames * ioChannels))
			{
				error = "HTDemucs input window is shorter than its configured shape.";
				return false;
			}

			interleavedOutput.assign(static_cast<size_t>(frames * ioChannels), 0.0f);

			for (int frame = 0; frame < frames; ++frame)
			{
				if (channels == 1)
					channelMajorInput[static_cast<size_t>(frame)]
						= 0.5f * (interleavedInput[static_cast<size_t>(frame * ioChannels)]
							+ interleavedInput[static_cast<size_t>(frame * ioChannels + 1)]);
				else
					for (int channel = 0; channel < channels; ++channel)
						channelMajorInput[static_cast<size_t>(channel * frames + frame)]
							= interleavedInput[static_cast<size_t>(frame * ioChannels + channel)];
			}

			auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
			auto inputTensor = Ort::Value::CreateTensor<float>(memoryInfo,
															   channelMajorInput.data(),
															   channelMajorInput.size(),
															   inputShape.data(),
															   inputShape.size());

			const char* inputNames[] = { inputName.c_str() };
			const char* outputNames[] = { outputName.c_str() };

			auto outputValues = session->Run(Ort::RunOptions{ nullptr },
											 inputNames,
											 &inputTensor,
											 1,
											 outputNames,
											 1);

			if (outputValues.empty() || !outputValues.front().IsTensor())
			{
				error = "HTDemucs inference returned no tensor output.";
				return false;
			}

			auto& outputTensor = outputValues.front();
			const auto outputShape = outputTensor.GetTensorTypeAndShapeInfo().GetShape();
			const auto* outputData = outputTensor.GetTensorData<float>();

			if (outputShape.size() == 4 && outputShape[0] == 1)
			{
				const auto stemCount = static_cast<int>(outputShape[1]);
				const auto outputChannels = static_cast<int>(outputShape[2]);
				const auto outputFrames = static_cast<int>(outputShape[3]);
				if (stemCount <= 0 || outputChannels <= 0 || outputFrames <= 0
					|| manifest.guitarStemIndex >= stemCount)
				{
					error = "HTDemucs output tensor has invalid dimensions or guitar stem index.";
					return false;
				}

				const auto guitarStemIndex = manifest.guitarStemIndex;
				const auto frameCountToCopy = juce::jmin(frames, outputFrames);
				const auto channelCountToCopy = juce::jmin(channels, outputChannels);

				for (int frame = 0; frame < frameCountToCopy; ++frame)
				{
					for (int channel = 0; channel < channelCountToCopy; ++channel)
					{
						const auto outputIndex = static_cast<size_t>(frame * ioChannels + channel);
						const auto tensorIndex = static_cast<size_t>((((0 * stemCount) + guitarStemIndex) * outputChannels + channel) * outputFrames + frame);
						interleavedOutput[outputIndex] = outputData[tensorIndex];
					}

					if (channels == 1)
						interleavedOutput[static_cast<size_t>(frame * ioChannels + 1)]
							= interleavedOutput[static_cast<size_t>(frame * ioChannels)];
					else if (outputChannels == 1)
						interleavedOutput[static_cast<size_t>(frame * ioChannels + 1)]
							= interleavedOutput[static_cast<size_t>(frame * ioChannels)];
				}

				return true;
			}

			if (outputShape.size() == 3 && outputShape[0] == 1)
			{
				const auto outputChannels = static_cast<int>(outputShape[1]);
				const auto outputFrames = static_cast<int>(outputShape[2]);
				if (outputChannels <= 0 || outputFrames <= 0)
				{
					error = "HTDemucs output tensor has invalid dimensions.";
					return false;
				}

				const auto frameCountToCopy = juce::jmin(frames, outputFrames);
				const auto channelCountToCopy = juce::jmin(channels, outputChannels);

				for (int frame = 0; frame < frameCountToCopy; ++frame)
				{
					for (int channel = 0; channel < channelCountToCopy; ++channel)
						interleavedOutput[static_cast<size_t>(frame * ioChannels + channel)]
							= outputData[static_cast<size_t>(channel * outputFrames + frame)];

					if (channels == 1)
						interleavedOutput[static_cast<size_t>(frame * ioChannels + 1)]
							= interleavedOutput[static_cast<size_t>(frame * ioChannels)];
					else if (outputChannels == 1)
						interleavedOutput[static_cast<size_t>(frame * ioChannels + 1)]
							= interleavedOutput[static_cast<size_t>(frame * ioChannels)];
				}

				return true;
			}

			error = "Unsupported HTDemucs output tensor shape.";
			return false;
		}
		catch (const Ort::Exception& exception)
		{
			error = "HTDemucs inference failed: " + juce::String(exception.what());
			return false;
		}
#else
		juce::ignoreUnused(interleavedInput, interleavedOutput, manifest);
		error = "ONNX Runtime support is not compiled into this build.";
		return false;
#endif
	}

private:
#if AUDIO_ANALYZER_HAS_ONNXRUNTIME
	Ort::AllocatorWithDefaultOptions allocator;
	std::unique_ptr<Ort::Env> env;
	std::unique_ptr<Ort::Session> session;
	std::array<int64_t, 3> inputShape{};
	std::string inputName;
	std::string outputName;
	std::vector<float> channelMajorInput;
#endif
};

//==============================================================================
guitarSeparator::guitarSeparator()
	: Thread("HTDemucs Guitar Separator")
{
	initialiseSeparator();
	startThread();
}

guitarSeparator::~guitarSeparator()
{
	signalThreadShouldExit();
	stopThread(5000);

	setSource(nullptr);
	releaseResources();
}

void guitarSeparator::initialiseSeparator()
{
	modelManifest.sampleRate = defaultModelSampleRate;
	modelManifest.channels = defaultModelChannels;
	modelManifest.windowFrames = defaultModelWindowFrames;
	resetProcessingState();
	configureFallbackFilters();
	updateBackendState(BackendState::idle, "HTDemucs initialising");
}

void guitarSeparator::prepareToPlay(int samplesPerBlockExpected, double newSampleRate)
{
	const ScopedLock sl(callbackLock);
	const ScopedLock processingGuard(processingLock);

	sampleRate = newSampleRate;
	blockSize = samplesPerBlockExpected;
	configureFallbackFilters();
	resetProcessingState();

	if (source != nullptr)
		source->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void guitarSeparator::configureFallbackFilters()
{
	for (int channel = 0; channel < ioChannels; ++channel)
	{
		highPassFilters[channel].reset();
		lowPassFilters[channel].reset();
		highPassFilters[channel].setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, fallbackHighPassHz));
		lowPassFilters[channel].setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, fallbackLowPassHz));
	}
}

void guitarSeparator::resetProcessingState()
{
	const ScopedLock queueGuard(queueLock);
	const auto windowFrames = getInputWindowFrames();
	inputCapacityFrames = juce::jmax(defaultInputCapacityFrames, juce::jmax(blockSize * 32, windowFrames * 2));
	outputCapacityFrames = juce::jmax(defaultOutputCapacityFrames, juce::jmax(blockSize * 32, windowFrames * 2));
	inputRing.assign(static_cast<size_t>(inputCapacityFrames * ioChannels), 0.0f);
	outputRing.assign(static_cast<size_t>(outputCapacityFrames * ioChannels), 0.0f);
	processInputWindow.assign(static_cast<size_t>(windowFrames * ioChannels), 0.0f);
	processOutputWindow.assign(static_cast<size_t>(windowFrames * ioChannels), 0.0f);
	modelInputWindow.clear();
	modelOutputWindow.clear();
	previousWindowTail.assign(static_cast<size_t>(getInputWindowHopFrames() * ioChannels), 0.0f);
	hasPreviousWindowTail = false;

	for (auto& scratch : fallbackChannelScratch)
		scratch.assign(static_cast<size_t>(windowFrames), 0.0f);

	inputWriteFrame = 0;
	inputReadFrame = 0;
	inputQueuedFrames = 0;
	outputWriteFrame = 0;
	outputReadFrame = 0;
	outputQueuedFrames = windowFrames;
	outputWriteFrame = outputQueuedFrames % outputCapacityFrames;
}

int guitarSeparator::getInputWindowFrames() const
{
	const auto modelRate = juce::jmax(1, modelManifest.sampleRate);
	const auto nativeRate = juce::jmax(1.0, sampleRate);
	return juce::jmax(2, juce::roundToInt(modelManifest.windowFrames * nativeRate / modelRate));
}

int guitarSeparator::getInputWindowHopFrames() const
{
	return juce::jmax(1, getInputWindowFrames() / 2);
}

void guitarSeparator::run()
{
	while (!threadShouldExit())
	{
		if (!modelAssetsChecked)
		{
			modelAssetsAvailable = ensureModelAssetsAvailable();
			modelAssetsChecked = true;
		}

		if (!backendReady.load() && modelAssetsAvailable)
			tryLoadHtDemucsBackend();

		if (!processAvailableWindow())
			wait(10);
	}
}

void guitarSeparator::getNextAudioBlock(const AudioSourceChannelInfo& info)
{
	const ScopedLock sl(callbackLock);

	auto numChans = info.buffer->getNumChannels();
	auto numSamples = info.numSamples;

	if ((source != nullptr) && (numChans > 0))
	{
		source->getNextAudioBlock(info);
		queueInputSamples(*info.buffer, info.startSample, numSamples);

		if (!popOutputSamples(*info.buffer, info.startSample, numSamples))
			info.clearActiveBufferRegion();
	}
	else
	{
		info.clearActiveBufferRegion();
	}
}

void guitarSeparator::releaseResources()
{
	const ScopedLock sl(callbackLock);
	const ScopedLock processingGuard(processingLock);
	resetProcessingState();
	configureFallbackFilters();

	if (source != nullptr)
		source->releaseResources();
}

void guitarSeparator::setSource(AudioSource* newSource)
{
	if (source != newSource)
	{
		auto* oldSource = source;

		const ScopedLock sl(callbackLock);

		if (newSource != nullptr && blockSize > 0 && sampleRate > 0)
			newSource->prepareToPlay(blockSize, sampleRate);

		source = newSource;

		if (oldSource != nullptr)
			oldSource->releaseResources();
	}
}

bool guitarSeparator::isModelReady() const
{
	return backendReady.load();
}

juce::String guitarSeparator::getBackendStatusText() const
{
	const ScopedLock sl(stateLock);
	return backendStatusText;
}

void guitarSeparator::updateBackendState(BackendState newState, const juce::String& newStatus)
{
	const ScopedLock sl(stateLock);
	backendState = newState;
	backendStatusText = newStatus;
	backendReady.store(newState == BackendState::ready);
}

juce::File guitarSeparator::getModelDirectory() const
{
	auto directory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
		.getChildFile("AudioAnalyzer")
		.getChildFile("Models")
		.getChildFile("HTDemucs");

	directory.createDirectory();
	return directory;
}

juce::File guitarSeparator::getModelFile() const
{
	return getModelDirectory().getChildFile(modelManifest.modelFileName);
}

juce::File guitarSeparator::getManifestFile() const
{
	return getModelDirectory().getChildFile(modelManifest.manifestFileName);
}

juce::String guitarSeparator::getConfiguredModelUrl() const
{
	return juce::SystemStats::getEnvironmentVariable(modelUrlEnvVar, {});
}

juce::String guitarSeparator::getConfiguredManifestUrl() const
{
	return juce::SystemStats::getEnvironmentVariable(manifestUrlEnvVar, {});
}

bool guitarSeparator::ensureModelAssetsAvailable()
{
	if (getModelFile().existsAsFile())
	{
		parseModelManifest();
		return true;
	}

	return downloadModelAssets();
}

bool guitarSeparator::downloadModelAssets()
{
	const auto modelUrlString = getConfiguredModelUrl();
	const auto manifestUrlString = getConfiguredManifestUrl();

	if (modelUrlString.isEmpty())
	{
		updateBackendState(BackendState::fallback,
			"HTDemucs model URL is not configured. Set AUDIO_ANALYZER_HTDEMUCS_MODEL_URL to enable model download.");
		return false;
	}

	updateBackendState(BackendState::downloadingModel, "Downloading HTDemucs model assets");

	if (!manifestUrlString.isEmpty())
		(void) downloadFileTo(juce::URL(manifestUrlString), getManifestFile());

	const auto modelDownloaded = downloadFileTo(juce::URL(modelUrlString), getModelFile());

	if (!modelDownloaded)
	{
		updateBackendState(BackendState::fallback,
			"Unable to download HTDemucs model. Using guitar-emphasis fallback backend.");
		return false;
	}

	parseModelManifest();
	return true;
}

bool guitarSeparator::parseModelManifest()
{
	const ScopedLock processingGuard(processingLock);
	const auto manifestFile = getManifestFile();
	if (!manifestFile.existsAsFile())
		return true;

	const auto parsedManifest = juce::JSON::parse(manifestFile.loadFileAsString());
	if (auto* object = parsedManifest.getDynamicObject())
	{
		juce::ignoreUnused(object);
		modelManifest.sampleRate = juce::jmax(8000, static_cast<int>(parsedManifest.getProperty("sampleRate", modelManifest.sampleRate)));
		modelManifest.channels = juce::jlimit(1, 2, static_cast<int>(parsedManifest.getProperty("channels", modelManifest.channels)));
		modelManifest.windowFrames = juce::jmax(blockSize * 2, static_cast<int>(parsedManifest.getProperty("windowFrames", modelManifest.windowFrames)));
		modelManifest.guitarStemIndex = juce::jmax(0, static_cast<int>(parsedManifest.getProperty("guitarStemIndex", modelManifest.guitarStemIndex)));

		resetProcessingState();
		configureFallbackFilters();
		return true;
	}

	updateBackendState(BackendState::fallback,
		"HTDemucs manifest could not be parsed. Using fallback backend defaults.");
	return false;
}

bool guitarSeparator::downloadFileTo(const juce::URL& url, const juce::File& destination) const
{
	auto temporaryFile = destination.getNonexistentSibling();
	temporaryFile.deleteFile();
	temporaryFile.create();

	auto inputStream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
		.withConnectionTimeoutMs(15000)
		.withNumRedirectsToFollow(4));

	if (inputStream == nullptr)
		return false;

	juce::FileOutputStream outputStream(temporaryFile);
	if (!outputStream.openedOk())
		return false;

	if (outputStream.writeFromInputStream(*inputStream, -1) <= 0)
		return false;

	outputStream.flush();
	destination.deleteFile();
	return temporaryFile.moveFileTo(destination);
}

bool guitarSeparator::tryLoadHtDemucsBackend()
{
	if (onnxBackend == nullptr)
		onnxBackend = std::make_unique<HtDemucsOnnxBackend>();

	juce::String loadError;
	if (onnxBackend->initialise(getModelFile(), modelManifest, loadError))
	{
		updateBackendState(BackendState::ready, "HTDemucs guitar separator ready");
		return true;
	}

	updateBackendState(BackendState::fallback, loadError + " Using guitar-emphasis fallback backend.");
	return false;
}

bool guitarSeparator::processAvailableWindow()
{
	const ScopedLock processingGuard(processingLock);
	if (!popInputWindow(processInputWindow))
		return false;

	processCurrentWindow(processOutputWindow);
	pushOutputWindow(processOutputWindow);
	return true;
}

bool guitarSeparator::processCurrentWindow(std::vector<float>& outputWindow)
{
	outputWindow.assign(processInputWindow.size(), 0.0f);

	if (backendReady.load() && onnxBackend != nullptr)
	{
		const auto nativeFrames = getInputWindowFrames();
		resampleInterleaved(processInputWindow, nativeFrames,
			modelInputWindow, modelManifest.windowFrames, ioChannels);

		juce::String processError;
		if (onnxBackend->processWindow(modelInputWindow, modelOutputWindow, modelManifest, processError))
		{
			resampleInterleaved(modelOutputWindow, modelManifest.windowFrames,
				outputWindow, nativeFrames, ioChannels);
			return true;
		}

		updateBackendState(BackendState::fallback,
			processError + " Falling back to guitar-emphasis filtering.");
	}

	processFallbackWindow(processInputWindow, outputWindow);
	return false;
}

void guitarSeparator::processFallbackWindow(const std::vector<float>& inputWindow,
	std::vector<float>& outputWindow)
{
	const auto frameCount = static_cast<int>(inputWindow.size() / ioChannels);
	const auto channelCount = ioChannels;

	for (int channel = 0; channel < channelCount; ++channel)
	{
		auto& scratch = fallbackChannelScratch[static_cast<size_t>(channel)];
		if (static_cast<int>(scratch.size()) != frameCount)
			scratch.assign(static_cast<size_t>(frameCount), 0.0f);
		highPassFilters[static_cast<size_t>(channel)].reset();
		lowPassFilters[static_cast<size_t>(channel)].reset();

		for (int frame = 0; frame < frameCount; ++frame)
			scratch[static_cast<size_t>(frame)] = inputWindow[static_cast<size_t>(frame * ioChannels + channel)];

		highPassFilters[static_cast<size_t>(channel)].processSamples(scratch.data(), frameCount);
		lowPassFilters[static_cast<size_t>(channel)].processSamples(scratch.data(), frameCount);
	}

	for (int frame = 0; frame < frameCount; ++frame)
	{
		float left = fallbackChannelScratch[0][static_cast<size_t>(frame)];
		float right = channelCount > 1 ? fallbackChannelScratch[1][static_cast<size_t>(frame)] : left;

		const auto mid = 0.5f * (left + right);
		const auto side = 0.5f * (left - right);
		left = (fallbackResidualGain * mid) + side;
		right = (fallbackResidualGain * mid) - side;

		if (std::abs(left) < fallbackGateThreshold)
			left *= fallbackResidualGain;
		if (std::abs(right) < fallbackGateThreshold)
			right *= fallbackResidualGain;

		outputWindow[static_cast<size_t>(frame * ioChannels)] = left;
		if (ioChannels > 1)
			outputWindow[static_cast<size_t>(frame * ioChannels + 1)] = right;
	}
}

void guitarSeparator::queueInputSamples(const juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples)
{
	const auto* left = buffer.getReadPointer(0, startSample);
	const auto* right = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1, startSample) : left;

	const ScopedLock sl(queueLock);

	for (int sample = 0; sample < numSamples; ++sample)
	{
		if (inputQueuedFrames >= inputCapacityFrames)
		{
			inputReadFrame = (inputReadFrame + 1) % inputCapacityFrames;
			--inputQueuedFrames;
		}

		const auto baseIndex = static_cast<size_t>(inputWriteFrame * ioChannels);
		inputRing[baseIndex] = left[sample];
		if (ioChannels > 1)
			inputRing[baseIndex + 1] = right[sample];

		inputWriteFrame = (inputWriteFrame + 1) % inputCapacityFrames;
		++inputQueuedFrames;
	}
}

bool guitarSeparator::popOutputSamples(juce::AudioBuffer<float>& buffer,
	int startSample,
	int numSamples)
{
	const ScopedLock sl(queueLock);
	if (outputQueuedFrames < numSamples)
		return false;

	auto* left = buffer.getWritePointer(0, startSample);
	auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1, startSample) : nullptr;

	for (int sample = 0; sample < numSamples; ++sample)
	{
		const auto baseIndex = static_cast<size_t>(outputReadFrame * ioChannels);
		left[sample] = outputRing[baseIndex];

		if (right != nullptr)
			right[sample] = outputRing[baseIndex + 1];

		outputReadFrame = (outputReadFrame + 1) % outputCapacityFrames;
		--outputQueuedFrames;
	}

	for (int channel = 2; channel < buffer.getNumChannels(); ++channel)
		buffer.copyFrom(channel, startSample, left, numSamples);

	return true;
}

bool guitarSeparator::popInputWindow(std::vector<float>& inputWindow)
{
	const ScopedLock sl(queueLock);
	const auto windowFrames = getInputWindowFrames();
	const auto hopFrames = getInputWindowHopFrames();
	if (inputQueuedFrames < windowFrames)
		return false;

	inputWindow.assign(static_cast<size_t>(windowFrames * ioChannels), 0.0f);

	for (int frame = 0; frame < windowFrames; ++frame)
	{
		const auto ringFrame = (inputReadFrame + frame) % inputCapacityFrames;
		const auto baseIndex = static_cast<size_t>(ringFrame * ioChannels);
		inputWindow[static_cast<size_t>(frame * ioChannels)] = inputRing[baseIndex];
		if (ioChannels > 1)
			inputWindow[static_cast<size_t>(frame * ioChannels + 1)] = inputRing[baseIndex + 1];

	}

	inputReadFrame = (inputReadFrame + hopFrames) % inputCapacityFrames;
	inputQueuedFrames -= hopFrames;
	return true;
}

void guitarSeparator::pushOutputWindow(const std::vector<float>& outputWindow)
{
	const ScopedLock sl(queueLock);
	const auto frameCount = juce::jmin(getInputWindowHopFrames(),
		static_cast<int>(outputWindow.size() / ioChannels) - getInputWindowHopFrames());

	for (int frame = 0; frame < frameCount; ++frame)
	{
		if (outputQueuedFrames >= outputCapacityFrames)
		{
			outputReadFrame = (outputReadFrame + 1) % outputCapacityFrames;
			--outputQueuedFrames;
		}

		const auto baseIndex = static_cast<size_t>(outputWriteFrame * ioChannels);
		const auto fadeIn = static_cast<float>(frame + 1) / frameCount;
		for (int channel = 0; channel < ioChannels; ++channel)
		{
			const auto current = outputWindow[static_cast<size_t>(frame * ioChannels + channel)];
			const auto previous = previousWindowTail[static_cast<size_t>(frame * ioChannels + channel)];
			outputRing[baseIndex + static_cast<size_t>(channel)]
				= hasPreviousWindowTail ? previous * (1.0f - fadeIn) + current * fadeIn : current;
			previousWindowTail[static_cast<size_t>(frame * ioChannels + channel)]
				= outputWindow[static_cast<size_t>((frame + frameCount) * ioChannels + channel)];
		}

		outputWriteFrame = (outputWriteFrame + 1) % outputCapacityFrames;
		++outputQueuedFrames;
	}

	hasPreviousWindowTail = true;
}

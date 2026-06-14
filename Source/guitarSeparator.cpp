/*
  ==============================================================================

	guitarSeparator.cpp
	Created: 24 Sep 2025 1:27:21am
	Author:  eigil

  ==============================================================================
*/

#include <JuceHeader.h>
#include "guitarSeparator.h"

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
constexpr int defaultInputCapacityFrames = 65536;
constexpr int defaultOutputCapacityFrames = 65536;
constexpr float fallbackHighPassHz = 70.0f;
constexpr float fallbackLowPassHz = 6000.0f;
constexpr float fallbackGateThreshold = 0.01f;
constexpr float fallbackResidualGain = 0.35f;
constexpr auto modelUrlEnvVar = "AUDIO_ANALYZER_HTDEMUCS_MODEL_URL";
constexpr auto manifestUrlEnvVar = "AUDIO_ANALYZER_HTDEMUCS_MANIFEST_URL";
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
			const auto frames = manifest.windowFrames;
			const auto channels = manifest.channels;

			channelMajorInput.assign(static_cast<size_t>(channels * frames), 0.0f);
			interleavedOutput.assign(interleavedInput.size(), 0.0f);

			for (int frame = 0; frame < frames; ++frame)
				for (int channel = 0; channel < channels; ++channel)
					channelMajorInput[static_cast<size_t>(channel * frames + frame)]
						= interleavedInput[static_cast<size_t>(frame * channels + channel)];

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

			if (outputShape.size() == 4)
			{
				const auto stemCount = static_cast<int>(outputShape[1]);
				const auto outputChannels = static_cast<int>(outputShape[2]);
				const auto outputFrames = static_cast<int>(outputShape[3]);
				const auto guitarStemIndex = juce::jlimit(0, stemCount - 1, manifest.guitarStemIndex);
				const auto frameCountToCopy = juce::jmin(frames, outputFrames);
				const auto channelCountToCopy = juce::jmin(channels, outputChannels);

				for (int frame = 0; frame < frameCountToCopy; ++frame)
				{
					for (int channel = 0; channel < channelCountToCopy; ++channel)
					{
						const auto outputIndex = static_cast<size_t>(frame * channels + channel);
						const auto tensorIndex = static_cast<size_t>((((0 * stemCount) + guitarStemIndex) * outputChannels + channel) * outputFrames + frame);
						interleavedOutput[outputIndex] = outputData[tensorIndex];
					}
				}

				return true;
			}

			if (outputShape.size() == 3 && outputShape[0] == 1)
			{
				const auto outputChannels = static_cast<int>(outputShape[1]);
				const auto outputFrames = static_cast<int>(outputShape[2]);
				const auto frameCountToCopy = juce::jmin(frames, outputFrames);
				const auto channelCountToCopy = juce::jmin(channels, outputChannels);

				for (int frame = 0; frame < frameCountToCopy; ++frame)
					for (int channel = 0; channel < channelCountToCopy; ++channel)
						interleavedOutput[static_cast<size_t>(frame * channels + channel)]
							= outputData[static_cast<size_t>(channel * outputFrames + frame)];

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
	inputCapacityFrames = juce::jmax(defaultInputCapacityFrames, blockSize * 32);
	outputCapacityFrames = juce::jmax(defaultOutputCapacityFrames, blockSize * 32);
	inputRing.assign(static_cast<size_t>(inputCapacityFrames * ioChannels), 0.0f);
	outputRing.assign(static_cast<size_t>(outputCapacityFrames * ioChannels), 0.0f);
	processInputWindow.assign(static_cast<size_t>(modelManifest.windowFrames * ioChannels), 0.0f);
	processOutputWindow.assign(static_cast<size_t>(modelManifest.windowFrames * ioChannels), 0.0f);

	for (auto& scratch : fallbackChannelScratch)
		scratch.assign(static_cast<size_t>(modelManifest.windowFrames), 0.0f);

	inputWriteFrame = 0;
	inputReadFrame = 0;
	inputQueuedFrames = 0;
	outputWriteFrame = 0;
	outputReadFrame = 0;
	outputQueuedFrames = 0;
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

		if (!popOutputSamples(*info.buffer, info.startSample, numSamples) && !preferDryWhileWarmingUp)
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
		juce::String processError;
		if (onnxBackend->processWindow(processInputWindow, outputWindow, modelManifest, processError))
			return true;

		updateBackendState(BackendState::fallback,
			processError + " Falling back to guitar-emphasis filtering.");
	}

	processFallbackWindow(processInputWindow, outputWindow);
	return false;
}

void guitarSeparator::processFallbackWindow(const std::vector<float>& inputWindow,
	std::vector<float>& outputWindow)
{
	const auto frameCount = modelManifest.windowFrames;
	const auto channelCount = juce::jlimit(1, ioChannels, modelManifest.channels);

	for (int channel = 0; channel < channelCount; ++channel)
	{
		auto& scratch = fallbackChannelScratch[static_cast<size_t>(channel)];
		if (static_cast<int>(scratch.size()) != frameCount)
			scratch.assign(static_cast<size_t>(frameCount), 0.0f);

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
	if (inputQueuedFrames < modelManifest.windowFrames)
		return false;

	inputWindow.assign(static_cast<size_t>(modelManifest.windowFrames * ioChannels), 0.0f);

	for (int frame = 0; frame < modelManifest.windowFrames; ++frame)
	{
		const auto baseIndex = static_cast<size_t>(inputReadFrame * ioChannels);
		inputWindow[static_cast<size_t>(frame * ioChannels)] = inputRing[baseIndex];
		if (ioChannels > 1)
			inputWindow[static_cast<size_t>(frame * ioChannels + 1)] = inputRing[baseIndex + 1];

		inputReadFrame = (inputReadFrame + 1) % inputCapacityFrames;
	}

	inputQueuedFrames -= modelManifest.windowFrames;
	return true;
}

void guitarSeparator::pushOutputWindow(const std::vector<float>& outputWindow)
{
	const ScopedLock sl(queueLock);
	const auto frameCount = juce::jmin(modelManifest.windowFrames, static_cast<int>(outputWindow.size() / ioChannels));

	for (int frame = 0; frame < frameCount; ++frame)
	{
		if (outputQueuedFrames >= outputCapacityFrames)
		{
			outputReadFrame = (outputReadFrame + 1) % outputCapacityFrames;
			--outputQueuedFrames;
		}

		const auto baseIndex = static_cast<size_t>(outputWriteFrame * ioChannels);
		outputRing[baseIndex] = outputWindow[static_cast<size_t>(frame * ioChannels)];
		if (ioChannels > 1)
			outputRing[baseIndex + 1] = outputWindow[static_cast<size_t>(frame * ioChannels + 1)];

		outputWriteFrame = (outputWriteFrame + 1) % outputCapacityFrames;
		++outputQueuedFrames;
	}
}

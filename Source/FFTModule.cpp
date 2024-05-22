/*
  ==============================================================================

	FFTModule.cpp
	Created: 20 May 2024 1:24:42pm
	Author:  eigil

  ==============================================================================
*/

#include "SpectrogramComponent.h"
#include "FFTModule.h"

FFTModule::FFTModule
(
	std::shared_ptr<AudioDeviceManager> SADM
	,
	std::shared_ptr<freqPlotModule> FPM
)
	: module_freqPlot(FPM)
	, deviceManager(*SADM)
{
	//setSize(700, 300);

	formatManager.registerBasicFormats();

	ptrSpectrogramComponent =
		std::make_shared<SpectrogramComponent>(formatManager, SADM, this, FPM);

	addAndMakeVisible(ptrSpectrogramComponent.get());
	setOpaque(true);

	forwardFFT
		= std::make_unique<dsp::FFT>(fftOrder);

}

FFTModule::~FFTModule()
{
	transportSource.setSource(nullptr);
	audioSourcePlayer.setSource(nullptr);
	deviceManager.removeAudioCallback(&audioSourcePlayer);
}



//===================
// FFTModule
//===================

void FFTModule::paint(Graphics& g)
{
	g.fillAll(Colours::black);

	g.setOpacity(1.0f);
}

void FFTModule::selectFile
(
	double maxFreq
	,
	juce::TextEditor* fftOrder__textEditor
	,
	juce::TextEditor* Nbr_Samples__textEditor
	,
	juce::Label* fftSizeNbr__label
	,
	juce::TextEditor* Sample_Freq__textEditor
)
{
	chooser.launchAsync
	(
		FileBrowserComponent::openMode
		|
		FileBrowserComponent::canSelectFiles
		,
		[
			this
				,
				maxFreq
				,
				fftOrder__textEditor
				,
				Nbr_Samples__textEditor
				,
				fftSizeNbr__label
				,
				Sample_Freq__textEditor
		]
		(const FileChooser& fc) /*mutable*/
	{
		if (fc.getURLResults().size() > 0)
		{
			auto u = fc.getURLResult();

			handleAudioResource
			(
				std::move(u)
				,
				maxFreq
				,
				fftOrder__textEditor
				,
				Nbr_Samples__textEditor
				,
				fftSizeNbr__label
				,
				Sample_Freq__textEditor
			);
		}
	}
	);
}

void FFTModule::handleAudioResource
(
	URL resource
	,
	double maxFreq
	,
	juce::TextEditor* fftOrder__textEditor
	,
	juce::TextEditor* Nbr_Samples__textEditor
	,
	juce::Label* fftSizeNbr__label
	,
	juce::TextEditor* Sample_Freq__textEditor
)
{
	if
		(
			!loadURLIntoFFT
			(
				resource
				,
				maxFreq
				,
				fftOrder__textEditor
				,
				Nbr_Samples__textEditor
				,
				fftSizeNbr__label
				,
				Sample_Freq__textEditor
			)
			)
	{
		// Failed to load the audio file!
		jassertfalse;
		return;
	}

	currentAudioFile = std::move(resource);
}

bool FFTModule::loadURLIntoFFT
(
	double maxFreq
	,
	juce::TextEditor* fftOrder__textEditor
	,
	juce::TextEditor* Nbr_Samples__textEditor
	,
	juce::Label* fftSizeNbr__label
	,
	juce::TextEditor* Sample_Freq__textEditor
)
{
	if (currentAudioFile != URL())
	{
		return loadURLIntoFFT
		(
			currentAudioFile
			,
			maxFreq
			,
			fftOrder__textEditor
			,
			Nbr_Samples__textEditor
			,
			fftSizeNbr__label
			,
			Sample_Freq__textEditor
		);
	}
	else
	{
		return false;
	}
}

bool FFTModule::loadURLIntoFFT
(
	const URL& audioURL
	,
	double maxFreq
	,
	juce::TextEditor* fftOrder__textEditor
	,
	juce::TextEditor* Nbr_Samples__textEditor
	,
	juce::Label* fftSizeNbr__label
	,
	juce::TextEditor* Sample_Freq__textEditor
)
{
	module_freqPlot->clearPlot();

	const auto source = makeInputSource(audioURL);

	if (source == nullptr)
		return false;

	auto stream = rawToUniquePtr(source->createInputStream());

	if (stream == nullptr)
		return false;

	reader = rawToUniquePtr(formatManager.createReaderFor(std::move(stream)));

	if (reader == nullptr)
		return false;

	juce::int64 bufferLengthInSamples = reader->lengthInSamples;
	unsigned int fftOrder = static_cast<unsigned int>(std::log2(bufferLengthInSamples));
	unsigned int fftSize = 1 << fftOrder;

	showValues
	(
		fftOrder
		,
		fftSize
		,
		static_cast<unsigned int>(reader->sampleRate)
		,
		fftOrder__textEditor
		,
		Nbr_Samples__textEditor
		,
		fftSizeNbr__label
		,
		Sample_Freq__textEditor
	);

	const unsigned int fftDataSize = fftSize << 1;

	float* fftData = new float[fftDataSize] { 0 };

	theAudioBuffer =
		std::make_unique<AudioBuffer<float>>
		(
			reader->numChannels
			,
			reader->lengthInSamples
		);

	reader->read
	(
		theAudioBuffer.get()
		,
		0
		,
		(int)reader->lengthInSamples
		,
		0
		,
		true
		,
		true
	);

	auto rdptrs = theAudioBuffer->getArrayOfReadPointers();

	memcpy(fftData, rdptrs[0], sizeof(float) * fftSize);

	juce::dsp::WindowingFunction<float> theHannWindow
	(
		fftSize
		,
		juce::dsp::WindowingFunction<float>::WindowingMethod::hann
	);

	theHannWindow.multiplyWithWindowingTable
	(
		fftData
		,
		fftSize
	);

	std::unique_ptr<dsp::FFT> forwardFFT =
		std::make_unique<dsp::FFT>(fftOrder);
	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	auto deltaHz = (float)reader->sampleRate / fftSize;

	std::vector<float> tmpFreqVctr(0);
	float freqVal = 0.0f;
	while (freqVal <= maxFreq)
	{
		tmpFreqVctr.push_back(freqVal);
		freqVal += deltaHz;
	}
	std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

	auto nbrSamplesInPlot = tmpFreqVctr.size();

	std::vector <std::vector<float>>
		plotValues{ { fftData, fftData + nbrSamplesInPlot } };

	cmp::GraphAttributeList graph_attributes(0);
	makeGraphAttributes(graph_attributes);

	cmp::StringVector plotLegend{ "p " + std::to_string(plotLegend.size() + 1) };

	module_freqPlot->setTitle("Frequency response [FFT]");
	module_freqPlot->setXLabel("[Hz]");
	module_freqPlot->setYLabel("[Magnitude]");

	module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

	return true;
}


void FFTModule::openAudioFile
(
	juce::ToggleButton* spectrumOfaudioFile__toggleButton
	, juce::ToggleButton* makespectrumOfInput__toggleButton
)
{
	chooser.launchAsync
	(
		FileBrowserComponent::openMode
		|
		FileBrowserComponent::canSelectFiles
		,
		[
			this
				, spectrumOfaudioFile__toggleButton
				, makespectrumOfInput__toggleButton
		]
		(const FileChooser& fc) /*mutable*/
		{
			if (fc.getURLResults().size() > 0)
			{
				juce::URL theUrl = fc.getURLResult();

				// Make spectrum plot
				ptrSpectrogramComponent->loadURLIntoSpectrum
				(
					theUrl
					, spectrumOfaudioFile__toggleButton
					, makespectrumOfInput__toggleButton
				);

				const auto source = makeInputSource(theUrl);
				auto stream = juce::rawToUniquePtr(source->createInputStream());
				auto reader =
					juce::rawToUniquePtr(formatManager.createReaderFor(std::move(stream)));

				if (reader.get() != nullptr)
				{
					currentAudioFileSource =
						std::make_unique<AudioFormatReaderSource>(reader.release(), true);

					transportSource.setSource
					(
						//currentAudioFileSource.get()
						//, 0			// tells it to buffer this many samples ahead
						//, nullptr	// this is the background thread to use for reading-ahead
						//, currentAudioFileSource->getAudioFormatReader()->sampleRate
						currentAudioFileSource.get()
						, 0   // tells it to buffer this many samples ahead
						, &thread // this is the background thread to use for reading-ahead
						, currentAudioFileSource->getAudioFormatReader()->sampleRate
					);
					audioSourcePlayer.setSource(&transportSource);

					currentAudioDevice = deviceManager.getCurrentAudioDevice();
					currentAudioDeviceType = deviceManager.getCurrentAudioDeviceType();
					currentDeviceTypeObject = deviceManager.getCurrentDeviceTypeObject();

					deviceManager.addAudioCallback(&audioSourcePlayer);
					transportSource.start();

					thread.startThread(Thread::Priority::normal);
				}
			}
		}
	);

}

void FFTModule::switchToMicrophoneInput()
{
	thread.stopThread(100);
	deviceManager.removeAudioCallback(&audioSourcePlayer);
	transportSource.stop();
	transportSource.setSource(nullptr);
	currentAudioFileSource.reset(nullptr);

	ptrSpectrogramComponent->switchToMicrophoneInput();
}

bool FFTModule::makeWhiteNoise
(
	unsigned int fftOrder
	,
	unsigned int fftSize
	,
	double maxFreq
	,
	unsigned int sampleRate
)
{
	module_freqPlot->clearPlot();

	const unsigned int fftDataSize = fftSize << 1;

	float* fftData = new float[fftDataSize] { 0 };

	juce::Random random;

	for (size_t i = 0; i < fftSize; i++)
	{
		fftData[i] = random.nextFloat() * 0.25f - 0.125f;
	}

	juce::dsp::WindowingFunction<float> theHannWindow
	(
		fftSize
		,
		juce::dsp::WindowingFunction<float>::WindowingMethod::hann
	);

	theHannWindow.multiplyWithWindowingTable
	(
		fftData
		,
		fftSize
	);

	std::unique_ptr<dsp::FFT> forwardFFT =
		std::make_unique<dsp::FFT>(fftOrder);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	auto deltaHz = (float)sampleRate / fftSize;

	std::vector<float> tmpFreqVctr(0);
	float freqVal = 0.0f;
	while (freqVal <= maxFreq)
	{
		tmpFreqVctr.push_back(freqVal);
		freqVal += deltaHz;
	}
	std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

	auto nbrSamplesInPlot = tmpFreqVctr.size();

	std::vector <std::vector<float>>
		plotValues{ { fftData, fftData + nbrSamplesInPlot } };

	cmp::GraphAttributeList graph_attributes(0);
	makeGraphAttributes(graph_attributes);

	cmp::StringVector plotLegend{ "p " + std::to_string(plotLegend.size() + 1) };

	module_freqPlot->setTitle("Frequency response [FFT]");
	module_freqPlot->setXLabel("[Hz]");
	module_freqPlot->setYLabel("[Magnitude]");

	module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

	return true;
}

bool FFTModule::makeSines
(
	unsigned int fftOrder
	,
	unsigned int fftSize
	,
	double maxFreq
	,
	unsigned int sampleRate
	,
	std::vector<double>& frequencies
)
{
	module_freqPlot->clearPlot();

	const unsigned int fftDataSize = fftSize << 1;

	float* fftData = new float[fftDataSize] { 0 };

	for (double freq : frequencies)
	{
		double currentPhase = 0.0f;
		auto cyclesPerSample = freq / sampleRate;
		double phaseDeltaPerSample =
			cyclesPerSample * juce::MathConstants<double>::twoPi;

		for (size_t i = 0; i < fftSize; i++)
		{
			fftData[i] += static_cast<float>(std::sin(currentPhase));
			currentPhase = std::fmod
			(
				currentPhase + phaseDeltaPerSample
				,
				juce::MathConstants<double>::twoPi
			);
		}
	}

	juce::dsp::WindowingFunction<float> theHannWindow
	(
		fftSize
		,
		juce::dsp::WindowingFunction<float>::WindowingMethod::hann
	);

	theHannWindow.multiplyWithWindowingTable
	(
		fftData
		,
		fftSize
	);

	std::unique_ptr<dsp::FFT> forwardFFT =
		std::make_unique<dsp::FFT>(fftOrder);
	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	auto deltaHz = (float)sampleRate / fftSize;

	std::vector<float> tmpFreqVctr(0);
	float freqVal = 0.0f;
	while (freqVal <= maxFreq)
	{
		tmpFreqVctr.push_back(freqVal);
		freqVal += deltaHz;
	}
	std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

	auto nbrSamplesInPlot = tmpFreqVctr.size();

	std::vector <std::vector<float>>
		plotValues{ { fftData, fftData + nbrSamplesInPlot } };

	cmp::GraphAttributeList graph_attributes(0);
	makeGraphAttributes(graph_attributes);

	cmp::StringVector plotLegend{ { "p " + std::to_string(plotLegend.size() + 1) } };

	module_freqPlot->setTitle("Frequency response [FFT]");
	module_freqPlot->setXLabel("[Hz]");
	module_freqPlot->setYLabel("[Magnitude]");

	module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

	return true;
}

bool FFTModule::makeWindows
(
	unsigned int fftOrder
	,
	unsigned int fftSize
	,
	double maxFreq
	,
	unsigned int sampleRate
)
{
	module_freqPlot->clearPlot();

	const unsigned int fftDataSize = fftSize << 1;

	// hann window
	float* fftData = new float[fftDataSize] { 0 };

	juce::dsp::WindowingFunction<float>::fillWindowingTables
	(
		fftData
		,
		fftSize
		,
		juce::dsp::WindowingFunction<float>::hann
	);

	std::unique_ptr<dsp::FFT> forwardFFT =
		std::make_unique<dsp::FFT>(fftOrder);

	// then render our FFT data..
	//forwardFFT->performFrequencyOnlyForwardTransform(fftData);
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	auto deltaHz = (float)sampleRate / fftSize;

	std::vector<float> tmpFreqVctr(0);
	float freqVal = 0.0f;
	while (freqVal <= maxFreq)
	{
		tmpFreqVctr.push_back(freqVal);
		freqVal += deltaHz;
	}
	std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

	auto nbrSamplesInPlot = tmpFreqVctr.size();

	std::vector <std::vector<float>>
		plotValues{ { fftData, fftData + nbrSamplesInPlot } };

	cmp::GraphAttributeList graph_attributes(0);
	makeGraphAttributes(graph_attributes);

	cmp::StringVector plotLegend{ "hann" };

	// hamming Window
	fftData = new float[fftDataSize] { 0 };

	juce::dsp::WindowingFunction<float>::fillWindowingTables
	(
		fftData
		,
		fftSize
		,
		juce::dsp::WindowingFunction<float>::hamming
	);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	frequencyValues.push_back(tmpFreqVctr);
	plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
	makeGraphAttributes(graph_attributes);
	plotLegend.push_back("hamming");

	// blackman Window
	fftData = new float[fftDataSize] { 0 };
	juce::dsp::WindowingFunction<float>::fillWindowingTables
	(
		fftData
		,
		fftSize
		,
		juce::dsp::WindowingFunction<float>::blackman
	);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	frequencyValues.push_back(tmpFreqVctr);
	plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
	makeGraphAttributes(graph_attributes);
	plotLegend.push_back("blackman");

	// blackmanHarris Window
	fftData = new float[fftDataSize] { 0 };
	juce::dsp::WindowingFunction<float>::fillWindowingTables
	(
		fftData
		,
		fftSize
		,
		juce::dsp::WindowingFunction<float>::blackmanHarris
	);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	frequencyValues.push_back(tmpFreqVctr);
	plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
	makeGraphAttributes(graph_attributes);
	plotLegend.push_back("blackmanHarris");

	// flatTop Window
	fftData = new float[fftDataSize] { 0 };
	juce::dsp::WindowingFunction<float>::fillWindowingTables
	(
		fftData
		,
		fftSize
		,
		juce::dsp::WindowingFunction<float>::flatTop
	);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	frequencyValues.push_back(tmpFreqVctr);
	plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
	makeGraphAttributes(graph_attributes);
	plotLegend.push_back("flatTop");

	// kaiser Window
	fftData = new float[fftDataSize] { 0 };
	juce::dsp::WindowingFunction<float>::fillWindowingTables
	(
		fftData
		,
		fftSize
		,
		juce::dsp::WindowingFunction<float>::kaiser
	);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

	frequencyValues.push_back(tmpFreqVctr);
	plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
	makeGraphAttributes(graph_attributes);
	plotLegend.push_back("kaiser");

	module_freqPlot->setTitle("Frequency response [FFT]");
	module_freqPlot->setXLabel("[Hz]");
	module_freqPlot->setYLabel("[Magnitude]");

	module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

	return true;
}

void FFTModule::makeGraphAttributes(cmp::GraphAttributeList& ga)
{
	cmp::GraphAttribute colourForLine;
	colourForLine.graph_colour = juce::Colour
	(
		static_cast<juce::int8>(randomRGB.nextInt(juce::Range(10, 255)))
		,
		static_cast<juce::int8>(randomRGB.nextInt(juce::Range(10, 255)))
		,
		static_cast<juce::int8>(randomRGB.nextInt(juce::Range(10, 255)))
	);
	ga.push_back(colourForLine);
}

void FFTModule::showValues
(
	unsigned int fftOrder
	,
	unsigned int fftSize
	,
	unsigned int sampleFreq
	,
	juce::TextEditor* fftOrder__textEditor
	,
	juce::TextEditor* Nbr_Samples__textEditor
	,
	juce::Label* fftSizeNbr__label
	,
	juce::TextEditor* Sample_Freq__textEditor
)
{
	fftOrder__textEditor->setText(String(fftOrder), true);
	Nbr_Samples__textEditor->setText(String(fftSize), true);
	fftSizeNbr__label->setText(String(fftSize), NotificationType::dontSendNotification);
	Sample_Freq__textEditor->setText(String(sampleFreq));
}

void FFTModule::setDoRealTimeFftChartPlot(bool doRTFftCP)
{
	ptrSpectrogramComponent->setDoRealTimeFftChartPlot(doRTFftCP);
}

void FFTModule::setMaxFreqInRealTimeFftChartPlot(double maxFRTFftCP)
{
	ptrSpectrogramComponent->setMaxFreqInRealTimeFftChartPlot(maxFRTFftCP);
}

std::shared_ptr<SpectrogramComponent> FFTModule::getPtrSpectrogramComponent()
{
	return ptrSpectrogramComponent;
}
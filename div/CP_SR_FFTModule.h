class FFTModule {
public:
    void registerObserver(FFTCtrl* observer) {
        this->observer = observer;
    }
    // ...
private:
    FFTCtrl* observer = nullptr;
    // ...
};

class SpectrogramComponent {
public:
    void registerObserver(FFTCtrl* observer) {
        this->observer = observer;
    }
    // ...
private:
    FFTCtrl* observer = nullptr;
    // ...
};
//////////////
void FFTModule::setSampleRate(double newSampleRate)
{
    this->sampleRate = newSampleRate;
    if (this->observer)
    {
        this->observer->updateSampleRate(this->sampleRate);
    }
}

void SpectrogramComponent::setSampleRate(double newSampleRate)
{
    this->sampleRate = newSampleRate;
    if (this->observer)
    {
        this->observer->updateSampleRate(this->sampleRate);
    }
}

class FFTCtrl
{
public:
    void updateSampleRate(double newSampleRate)
    {
        // Update the TextEditor or Label with the new sample rate
        Sample_Freq__textEditor->setText(juce::String(newSampleRate), juce::NotificationType::dontSendNotification);
    }
    // ...
};
FFTModule->registerObserver(this);
SpectrogramComponent->registerObserver(this);

class FFTModule
{
public:
    void registerObserver(FFTCtrl* observer)
    {
        this->observer = observer;
    }
    // ...
private:
    FFTCtrl* observer = nullptr;
    // ...
};

class SpectrogramComponent
{
public:
    void registerObserver(FFTCtrl* observer)
    {
        this->observer = observer;
    }
    // ...
private:
    FFTCtrl* observer = nullptr;
    // ...
};


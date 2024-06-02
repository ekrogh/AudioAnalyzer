private:
    std::unique_ptr<juce::Timer> yLimTimer;

///////////////////////////////
class YLimTimer : public juce::Timer
{
public:
    YLimTimer(SpectrogramComponent& component) : component(component) {}

    void timerCallback() override
    {
        component.calculateYLim();
    }

private:
    SpectrogramComponent& component;
};


///////////////////////////////
SpectrogramComponent::SpectrogramComponent(/* parameters */)
    : /* initialization */
{
    // other code...
    yLimTimer = std::make_unique<YLimTimer>(*this);
}

//////////////////////////
void SpectrogramComponent::calculateYLim()
{
    // Put your yLim calculation code here
}

//////////////////////////
SpectrogramComponent::~SpectrogramComponent()
{
    yLimTimer->stopTimer();
}

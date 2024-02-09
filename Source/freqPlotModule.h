/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.9

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "cmp_plot.h"
#include "example_utils.h"
#include <JuceHeader.h>
//[/Headers]



//==============================================================================
/**
																	//[Comments]
	An auto-generated component, created by the Projucer.

	Describe your class and how it works here!
																	//[/Comments]
*/
class freqPlotModule : public juce::Component
{
public:
	//==============================================================================
	freqPlotModule();
	~freqPlotModule() override;

	//==============================================================================
	//[UserMethods]     -- You can add your own custom methods in this section.
	void updatePlot
	(
		std::vector <std::vector<float>> y_values
		,
		std::vector <std::vector<float>> x_values
		,
		cmp::GraphAttributeList graph_attributes
		,
		cmp::StringVector& legend
	);
	void updatePlotRealTime(std::vector <std::vector<float>> y_values);
	void clearPlot();
	void setXLabel(const std::string& x_label) { m_plot.setXLabel(x_label); };
	void setYLabel(const std::string& y_label) { m_plot.setYLabel(y_label); };
	void setTitle(const std::string& title) { m_plot.setTitle(title); };
	//[/UserMethods]

	void paint(juce::Graphics& g) override;
	void resized() override;



private:
	//[UserVariables]   -- You can add your own custom variables in this section.
	cmp::Plot m_plot;
	//[/UserVariables]

	//==============================================================================


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(freqPlotModule)
};

//[EndFile] You can add extra defines here...
//[/EndFile]


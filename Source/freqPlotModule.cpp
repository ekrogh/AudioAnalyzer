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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "freqPlotModule.h"
#include "example_utils.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
freqPlotModule::freqPlotModule()
	: m_plot{ cmp::Plot() }
{
	//[Constructor_pre] You can add your own custom stuff here..
	addAndMakeVisible(m_plot);
	//[/Constructor_pre]


	//[UserPreSize]
	//[/UserPreSize]

	setSize(1000, 1000);


	//[Constructor] You can add your own custom stuff here..

	// Always plot atleast ones before calling realTimePlot
	m_plot.plot(
		{ cmp::generateSineWaveVector<float>((1 << 10), -1.0f, 1.0f, 1, 1) });

	//[/Constructor]
}

freqPlotModule::~freqPlotModule()
{
	//[Destructor_pre]. You can add your own custom destruction code here..
	//[/Destructor_pre]



	//[Destructor]. You can add your own custom destruction code here..
	//[/Destructor]
}

//==============================================================================
void freqPlotModule::paint(juce::Graphics& g)
{
	//[UserPrePaint] Add your own custom painting code here..
	//[/UserPrePaint]

	g.fillAll(juce::Colour(0xff505050));

	//[UserPaint] Add your own custom painting code here..
	//[/UserPaint]
}

void freqPlotModule::resized()
{
	//[UserPreResize] Add your own custom resize code here..
	// Set the bounds of the plot to fill the whole window.
	juce::Rectangle curBounds = getBounds();
	curBounds.setHeight(curBounds.getHeight() - 10);
	m_plot.setBounds(curBounds);
	//[/UserPreResize]

	//[UserResized] Add your own custom resize handling here..
	//[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void freqPlotModule::xLim(const float min, const float max)
{
	m_plot.xLim(min, max);
	
	//MessageManager::callAsync
	//(
	//	[this, min, max]
	//	()
	//	{
	//		m_plot.xLim(min, max);
	//	}
	//);
}

void freqPlotModule::yLim(const float min, const float max)
{
	//{
	//	MessageManagerLock ml (Thread::getCurrentThread());
	//	m_plot.yLim(min, max);
	//}
	MessageManager::callAsync
	(
		[this, min, max]
		()
		{
			m_plot.yLim(min, max);
			m_plot.repaint();
		}
	);
}

void freqPlotModule::updatePlot
(
	std::vector <std::vector<float>> y_values
	,
	std::vector <std::vector<float>> x_values
)
{
	MessageManager::callAsync([this]() { clearPlot(); });
	MessageManager::callAsync
	(
		[
			this
				,
				y_values
				,
				x_values
		]
		()
		{
			m_plot.plot(y_values, x_values);
		}
	);
}

void freqPlotModule::updatePlot
(
	std::vector <std::vector<float>> y_values
	,
	std::vector <std::vector<float>> x_values
	,
	cmp::GraphAttributeList graph_attributes
	,
	cmp::StringVector legend
)
{
	MessageManager::callAsync([this]() { clearPlot(); });
	MessageManager::callAsync
	(
		[
			this
				,
				y_values
				,
				x_values
				,
				graph_attributes
				,
				legend
		]
		()
		{
			m_plot.plot(y_values, x_values, graph_attributes);
			m_plot.setLegend(legend);
		}
	);
}


void freqPlotModule::updatePlotRealTime(std::vector <std::vector<float>> y_values)
{
	//m_plot.realTimePlot(y_values);
	MessageManager::callAsync
	(
		[this, y_values]
		()
		{
			m_plot.realTimePlot(y_values);
		}
	);
}

//void freqPlotModule::updatePlotRealTime
//(
//	std::vector <std::vector<float>> y_values
//	,
//	std::vector <std::vector<float>> x_values
//)
//{
//	m_plot.realTimePlot(y_values, x_values);
//	//MessageManager::callAsync
//	//(
//	//	[ this, y_values, x_values]
//	//	()
//	//	{
//	//		m_plot.realTimePlot(y_values, x_values);
//	//	}
//	//);
//}

void freqPlotModule::setXTicks(const std::vector<float>& x_ticks)
{
	m_plot.setXTicks(x_ticks);

	//MessageManager::callAsync
	//(
	//	[this, x_ticks]
	//	()
	//	{
	//		m_plot.setXTicks(x_ticks);
	//	}
	//);
}

void freqPlotModule::setXTickLabels(const std::vector<std::string>& x_labels)
{
	m_plot.setXTickLabels(x_labels);

	//MessageManager::callAsync
	//(
	//	[this, x_labels]
	//	()
	//	{
	//		m_plot.setXTickLabels(x_labels);
	//	}
	//);
}

void freqPlotModule::setYTicks(const std::vector<float>& x_ticks)
{
	m_plot.setYTicks(x_ticks);

	//MessageManager::callAsync
	//(
	//	[this, x_ticks]
	//	()
	//	{
	//		m_plot.setXTicks(x_ticks);
	//	}
	//);
}

void freqPlotModule::setYTickLabels(const std::vector<std::string>& x_labels)
{
	m_plot.setYTickLabels(x_labels);

	//MessageManager::callAsync
	//(
	//	[this, x_labels]
	//	()
	//	{
	//		m_plot.setXTickLabels(x_labels);
	//	}
	//);
}

void freqPlotModule::plotVerticalLines
(
	const std::vector<float>& x_coordinates
	,
	const cmp::GraphAttributeList& graph_attributes
)
{
	//m_plot.realTimePlot(y_values);
	MessageManager::callAsync
	(
		[this, x_coordinates, graph_attributes]
		()
		{
			m_plot.plotVerticalLines(x_coordinates, graph_attributes);
		}
	);
}

void freqPlotModule::clearPlot()
{
	m_plot.plot({ { 1 } }, { { 1 } });
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

	This is where the Projucer stores the metadata that describe this GUI layout, so
	make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="freqPlotModule" componentName=""
				 parentClasses="public juce::Component" constructorParams="" variableInitialisers="m_plot{cmp::Plot()}"
				 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
				 fixedSize="0" initialWidth="1000" initialHeight="1000">
  <BACKGROUND backgroundColour="ff505050"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]


#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


class SimpleThumbnailComponent : public juce::Component,
                                 private juce::ChangeListener
{
public:
    SimpleThumbnailComponent (int sourceSamplesPerThumbnailSample,
                              juce::AudioFormatManager& formatManager,
                              juce::AudioThumbnailCache& cache)
       : thumbnail (sourceSamplesPerThumbnailSample, formatManager, cache)
    {
        thumbnail.addChangeListener (this);
    }

    void setFile (const juce::File file)
    {
        fileCache = file;
        thumbnail.setSource (new juce::FileInputSource (file));
    }

    juce::File getFile ()
    {
        return fileCache;
    }
    
    void paint (juce::Graphics& g) override
    {
        if (thumbnail.getNumChannels() == 0)
            paintIfNoFileLoaded (g);
        else
            paintIfFileLoaded (g);
    }

    void paintIfNoFileLoaded (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::white);
        g.setColour (juce::Colours::darkgrey);
        g.drawFittedText ("No File Loaded - v6", getLocalBounds(), juce::Justification::centred, 1);
    }

    void paintIfFileLoaded (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::white);

        g.setColour (juce::Colours::red);
        thumbnail.drawChannels (g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f);
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            thumbnailChanged();
    }

private:
    void thumbnailChanged()
    {
        repaint();
    }

    juce::AudioThumbnail thumbnail;
    juce::File fileCache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleThumbnailComponent)
};

//------------------------------------------------------------------------------

class SimplePositionOverlay : public juce::Component,
                              private juce::Timer
{
public:
    SimplePositionOverlay (juce::AudioTransportSource* transportSourceToUse)
       : transportSource (transportSourceToUse)
    {
        startTimer (40);
    }

    void paint (juce::Graphics& g) override
    {
        if (transportSource == nullptr)
            return;
        
        auto duration = (float) transportSource->getLengthInSeconds();

        if (duration > 0.0)
        {
            auto audioPosition = (float) transportSource->getCurrentPosition();
            auto drawPosition = (audioPosition / duration) * (float) getWidth();

            g.setColour (juce::Colours::green);
            g.drawLine (drawPosition, 0.0f, drawPosition, (float) getHeight(), 2.0f);
        }
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        if (transportSource == nullptr)
            return;
        
        auto duration = transportSource->getLengthInSeconds();

        if (duration > 0.0)
        {
            auto clickPosition = event.position.x;
            auto audioPosition = (clickPosition / (float) getWidth()) * duration;

            transportSource->setPosition (audioPosition);
        }
    }

private:
    void timerCallback() override
    {
        repaint();
    }

    juce::AudioTransportSource* transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplePositionOverlay)
};

//------------------------------------------------------------------------------


const int kPortNumber = 8080;

class CustomConnection : public juce::InterprocessConnection
{
public:
    CustomConnection() : InterprocessConnection(true, 0)
    {
    }

    void connectionMade() override
    {
        printf("Connection made\n");
    }

    void connectionLost() override
    {
        printf("Connection lost\n");
    }

    void messageReceived(const juce::MemoryBlock& msg) override
    {
        const auto str = msg.toString();
        printf("From client: %s\n", str.toRawUTF8());
    }

private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomConnection);
};






//==============================================================================
/**
*/
class AmadeusAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::ChangeListener
{
public:
    AmadeusAudioProcessorEditor (AmadeusAudioProcessor&, juce::AudioTransportSource*);
    ~AmadeusAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AmadeusAudioProcessor& audioProcessor;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState (TransportState newState)
    {
        if (transportSource == nullptr)
            return;
        
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:
                    stopButton.setEnabled (false);
                    playButton.setEnabled (true);
                    transportSource->setPosition (0.0);
                    break;

                case Starting:
                    playButton.setEnabled (false);
                    transportSource->start();
                    break;

                case Playing:
                    stopButton.setEnabled (true);
                    break;

                case Stopping:
                    transportSource->stop();
                    break;

                default:
                    jassertfalse;
                    break;
            }
        }
    }

    void transportSourceChanged()
    {
        if (transportSource->isPlaying())
            changeState (Playing);
        else
            changeState (Stopped);
    }

    void openButtonClicked()
    {
        juce::FileChooser chooser ("Select a Wave file to play...",
                                   {},
                                   "*.wav;*.mp3");

        if (chooser.browseForFileToOpen())
        {
            juce::File file = chooser.getResult();

            if (auto* reader = formatManager.createReaderFor (file))
            {
                if (transportSource == nullptr)
                    transportSource = new juce::AudioTransportSource();
                std::unique_ptr<juce::AudioFormatReaderSource> newSource (new juce::AudioFormatReaderSource (reader, true));
                transportSource->setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                transportSource->addChangeListener (this);
                playButton.setEnabled (true);
                uploadButton.setEnabled (true);
                thumbnailComp.setFile (file);
                readerSource.reset (newSource.release());
            }
        }
        else
        {
            uploadButton.setEnabled (false);
            playButton.setEnabled (false);
        }
    }

    void playButtonClicked()
    {
        changeState (Starting);
    }

    void stopButtonClicked()
    {
        changeState (Stopping);
    }

    void uploadButtonClicked()
    {
        bool success = customConn.connectToSocket("127.0.0.1", 8080, 10000);
        DBG("Connecting to socket");
        DBG(std::to_string(success));
        juce::File file = thumbnailComp.getFile();
        juce::String filePath = file.getFullPathName();
        juce::String message("<:COMMAND:>PRACTICE<:FILEPATH:>" + filePath);
        juce::MemoryBlock mb(message.toRawUTF8(), message.length());
        customConn.sendMessage(mb);
        customConn.disconnect();
    }

    //==========================================================================
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton uploadButton;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    TransportState state;
    juce::AudioTransportSource* transportSource;
    juce::AudioThumbnailCache thumbnailCache;
    SimpleThumbnailComponent thumbnailComp;
    SimplePositionOverlay positionOverlay;
    CustomConnection customConn;
    
    friend class PluginProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmadeusAudioProcessorEditor)
};

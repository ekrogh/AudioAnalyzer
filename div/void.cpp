void openAudioFile(File audioFile)
{
    // Create a new audio format reader for the file
    std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(audioFile));

    if (reader)
    {
        // Create a new AudioFormatReaderSource for the reader
        auto newSource = std::make_unique<AudioFormatReaderSource>(reader.release(), true);

        // Set the source for the AudioSourcePlayer
        audioSourcePlayer.setSource(newSource.get());

        // Take ownership of the new source
        currentAudioSource.reset(newSource.release());
    }
}


void openAudioFile()
{
    // ... (existing code) ...

    if (reader.get() != nullptr)
    {
        currentAudioFileSource =
            std::make_unique<AudioFormatReaderSource>(reader.release(), true);

        transportSource.setSource
        (
            currentAudioFileSource.get()
            , 0         // tells it to buffer this many samples ahead
            , nullptr   // this is the background thread to use for reading-ahead
            , currentAudioFileSource->getAudioFormatReader()->sampleRate
        );
        setSource(&transportSource);
        audioSourcePlayer.setSource(&transportSource);

        // Remove this as an audio callback from the deviceManager
        deviceManager.removeAudioCallback(this);

        deviceManager.addAudioCallback(&audioSourcePlayer);
        transportSource.start();
    }

    // ... (existing code) ...
}

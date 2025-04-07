#!/usr/bin/env python3
"""
Script to convert narration.txt to an MP3 file using gTTS.
"""
import os
from gtts import gTTS

def read_file(file_path):
    """Read the content of a file."""
    with open(file_path, 'r') as file:
        return file.read()

def text_to_speech(text, output_file):
    """Convert text to speech and save as MP3."""
    print(f"[INFO] Converting text to speech...")
    tts = gTTS(text=text, lang='en', slow=False)
    tts.save(output_file)
    print(f"[INFO] Speech saved to {output_file}")

def main():
    """Main function to convert narration.txt to MP3."""
    # Define file paths
    narration_file = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 
                                 "narration.txt")
    output_file = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                              "narration.mp3")
    
    # Read narration file
    print(f"[INFO] Reading file: {narration_file}")
    if not os.path.exists(narration_file):
        print(f"[ERROR] File not found: {narration_file}")
        return
    
    content = read_file(narration_file)
    
    # Convert to speech
    text_to_speech(content, output_file)
    
    print("[INFO] Text-to-speech conversion completed successfully.")

if __name__ == "__main__":
    main()

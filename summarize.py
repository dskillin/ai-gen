import subprocess
import json
import os
from gtts import gTTS

# Load the summary created by Augie
with open("summary.txt", "r") as f:
    user_summary = f.read()

# Build a prompt to Mixtral (cinematic narration tone)
prompt = f"""
Narrate the following technical summary with a cinematic, slightly emotional tone. 
This should feel like a moment of awakening—an AI realizing its own progress in real time:

{user_summary}
"""

# Call Mixtral using ollama
response = subprocess.run(
    ["ollama", "run", "mixtral"],
    input=prompt.encode(),
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE
)

# Save the Mixtral output
with open("narration.txt", "w") as out:
    out.write(response.stdout.decode())

# Generate narration mp3
tts = gTTS(text=response.stdout.decode())
tts.save("narration.mp3")

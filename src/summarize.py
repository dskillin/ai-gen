#!/usr/bin/env python3
"""
Script to summarize the tasks.md file using Ollama's mistral model.
"""
import os
import json
import subprocess
import requests

def read_file(file_path):
    """Read the content of a file."""
    with open(file_path, 'r') as file:
        return file.read()

def save_file(file_path, content):
    """Save content to a file."""
    with open(file_path, 'w') as file:
        file.write(content)
    print(f"Content saved to {file_path}")

def summarize_with_ollama(text):
    """Use Ollama API to summarize text with mistral model."""
    print("[INFO] Requesting summary from Ollama (mistral model)...")
    
    url = "http://localhost:11434/api/generate"
    payload = {
        "model": "mistral",
        "prompt": f"Please summarize the following text in a concise, clear manner. Focus on the main tasks and their status:\n\n{text}",
        "stream": False
    }
    
    try:
        response = requests.post(url, json=payload)
        response.raise_for_status()
        result = response.json()
        return result.get("response", "No response received")
    except requests.exceptions.RequestException as e:
        print(f"[ERROR] Failed to get summary: {e}")
        return f"Error: {e}"

def main():
    """Main function to summarize tasks.md and save to narration.txt."""
    # Define file paths
    tasks_file = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 
                             ".augie", "tasks.md")
    output_file = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                              "narration.txt")
    
    # Read tasks file
    print(f"[INFO] Reading file: {tasks_file}")
    if not os.path.exists(tasks_file):
        print(f"[ERROR] File not found: {tasks_file}")
        return
    
    content = read_file(tasks_file)
    
    # Get summary from Ollama
    summary = summarize_with_ollama(content)
    
    # Save summary to narration.txt
    save_file(output_file, summary)
    print("[INFO] Summary generated and saved to narration.txt")

if __name__ == "__main__":
    main()

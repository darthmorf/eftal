@echo off
wsl -d Ubuntu --cd "%CD%" -- bash -ic "claude --resume; exec bash"
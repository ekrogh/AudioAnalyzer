import yaml

# Load the model configuration from the YAML file
with open('D:/Users/eigil/projects/pythonProjs/demucs/demucs/remote/htdemucs_6s.yaml', 'r') as f:
    config = yaml.safe_load(f)

# Print the keys in the configuration file
print("Configuration keys:", config.keys())

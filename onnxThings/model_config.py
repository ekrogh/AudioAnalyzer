import yaml

# Load the model configuration from the YAML file
with open('D:/Users/eigil/projects/pythonProjs/demucs/demucs/remote/htdemucs_6s.yaml', 'r') as f:
    config = yaml.safe_load(f)

# Extract the configuration for the model from the 'models' key
model_config = config['models'][0]  # Assuming the first model in the list is the target

# Print the contents of model_config
print("Model Configuration:", model_config)

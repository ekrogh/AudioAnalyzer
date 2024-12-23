import torch
from demucs import pretrained
from demucs.apply import apply_model, tensor_chunk

def separate_guitar_buffer(buffer):
    input_tensor = torch.tensor(buffer, dtype=torch.float32)

    # Debug: Print initial input tensor shape, type, and first few values
    print(f"Initial input tensor shape: {input_tensor.shape}")
    print(f"Initial input tensor dtype: {input_tensor.dtype}")
    print(f"Initial input tensor values: {input_tensor[:10]}")

    # Check and fix dimensions if necessary
    if len(input_tensor.shape) == 1:
        input_tensor = input_tensor.unsqueeze(0)  # Add batch dimension
    if len(input_tensor.shape) == 2:
        input_tensor = input_tensor.unsqueeze(1)  # Add channel dimension if not present

    # Ensure tensor has 2 channels (stereo)
    if input_tensor.shape[1] == 1:
        input_tensor = input_tensor.repeat(1, 2, 1)

    # Ensure the tensor length is appropriate for chunking
    chunk_size = 512  # Example chunk size used by Demucs
    total_length = input_tensor.shape[-1]

    # Debug: Print total length before padding
    print(f"Total length before padding: {total_length}")

    # Calculate necessary padding
    pad_size = chunk_size - (total_length % chunk_size) if total_length % chunk_size != 0 else 0
    if pad_size > 0:
        input_tensor = torch.nn.functional.pad(input_tensor, (0, pad_size))

    total_length = input_tensor.shape[-1]  # Update total length

    # Debug: Print total length after padding
    print(f"Total length after padding: {total_length}")

    # Ensure offsets are valid
    offset = 0
    print(f"Checking offset: {offset} within total length: {total_length}")
    assert offset >= 0 and offset < total_length, "Invalid offset."

    length = total_length - offset
    print(f"Calculated length: {length}")
    assert length > 0, "Invalid length."

    # Debug: Print final input tensor shape and values
    print(f"Final input tensor shape: {input_tensor.shape}")
    print(f"Final input tensor values: {input_tensor[:, :, :10]}")

    # Validate tensor dimensions and properties
    try:
        if offset < 0 or offset >= total_length:
            raise ValueError(f"Invalid offset: {offset}. Must be between 0 and {total_length - 1}")
        if length <= 0:
            raise ValueError(f"Invalid length: {length}. Must be greater than 0")
    except ValueError as e:
        print(f"Validation error: {e}")
        raise

    # Convert to TensorChunk
    try:
        input_chunk = tensor_chunk(input_tensor)
        print(f"Created TensorChunk with shape: {input_chunk.tensor.shape}, offset: {input_chunk.offset}, length: {input_chunk.length}")
    except AssertionError as e:
        print(f"AssertionError during TensorChunk creation: {str(e)}")
        raise

    # Load the pretrained Demucs model
    model = pretrained.get_model('htdemucs_6s')

    # Ensure model is in evaluation mode
    model.eval()

    # Apply the model to the TensorChunk
    try:
        print("Applying model to input tensor...")
        with torch.no_grad():
            separated = apply_model(model, input_chunk)
        print("Model applied successfully.")
    except AssertionError as e:
        print(f"AssertionError during model application: {str(e)}")
        raise
    except Exception as e:
        print(f"Unexpected error during model application: {e}")
        raise

    # Extract the guitar track from the separated output
    try:
        guitar = separated[0, 0].cpu().numpy()
        print("Guitar track extracted successfully.")
    except Exception as e:
        print(f"Error during extracting guitar track: {e}")
        raise

    return guitar

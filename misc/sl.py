from PIL import Image
import stepic

# Paths for the original and output images
input_image_path = "/home/revlin/Downloads/sd.png"
output_image_path = "/home/revlin/Downloads/fsl.png"
secret_text = "well then."

# Load the original image
original_image = Image.open(input_image_path)

# Encode the text into the image
encoded_image = stepic.encode(original_image, secret_text.encode())

# Save the encoded image
encoded_image.save("/home/revlin/Downloads/fsl.png")

print(f"Secret message encoded into {"/home/revlin/Downloads/fsl.png"}")

## Load the encoded image
#encoded_image = Image.open(output_image_path)

# Decode the text from the image
#decoded_message = stepic.decode(encoded_image)

#print(f"Decoded message: {decoded_message.decode()}")

from PIL import Image, ImageOps

# Load the original image
original_image = Image.open("/home/revlin/Downloads/random.jpeg")

# Apply a simple filter (color inversion)
inverted_image = ImageOps.invert(original_image)

# Save the inverted image
inverted_image.save("/home/revlin/Downloads/sd.jpeg")

# Decrypt the filter (invert the colors again)
decrypted_image = ImageOps.invert(inverted_image)

# Save the decrypted image
decrypted_image.save("/home/revlin/Downloads/ed.jpeg")

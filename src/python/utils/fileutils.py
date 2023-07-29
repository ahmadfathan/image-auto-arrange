from os import listdir

def get_image_files(dir, extensions=["png", "PNG"]):
    files = listdir(dir)
    image_files = []

    for file in files: 
        for extension in extensions:
            if file.endswith(extension): 
                image_files.append(file)
                
    return image_files

import cv2
import numpy as np

def cart2pol(x,y):
    theta = np.arctan2(y,x)
    rho = np.hypot(x,y)
    return theta, rho

def pol2cart(theta, rho):
    x = rho * np.cos(theta)
    y = rho * np.sin(theta)
    return x, y

def rotate_contour(cnt, angle):
    M = cv2.moments(cnt)
    cx = int(M['m10']/M['m00'])    
    cy = int(M['m01']/M['m00'])

    cnt_norm = cnt - [cx, cy]

    coordinates = cnt_norm[:,0,:]
    xs, ys = coordinates[:,0], coordinates[:,1]
    thetas, rhos = cart2pol(xs, ys)

    thetas = np.rad2deg(thetas)
    thetas = (thetas + angle) % 360
    thetas = np.deg2rad(thetas)

    xs, ys = pol2cart(thetas, rhos)

    cnt_norm[:,0,0] = xs
    cnt_norm[:,0,1] = ys

    cnt_rotated = cnt_norm + [cx, cy]
    cnt_rotated = cnt_rotated.astype(np.int32)

    return cnt_rotated


def scale_contour(cnt, scale):
    M = cv2.moments(cnt)
    cx = int(M['m10']/M['m00'])    
    cy = int(M['m01']/M['m00'])

    cnt_norm = cnt - [cx, cy]   
    cnt_scaled = cnt_norm * scale
    cnt_scaled = cnt_scaled + [cx, cy]
    cnt_scaled = cnt_scaled.astype(np.int32)

    return cnt_scaled


def get_contour_from_image(img):
    '''
        Get biggest contour from image
    '''

    img = img[:,:,3:] # Get alpha channel only

    # Creating the kernel with numpy
    kernel2 = np.ones((151, 151), np.float32)/25
    
    # Applying the filter
    blur = cv2.filter2D(src=img, ddepth=-1, kernel=kernel2)

    contours, _ = cv2.findContours(blur, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    return contours[0]
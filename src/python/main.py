from imgproc.contour import *
from ga.population import *
from ga.individual import *
from utils.fileutils import *
from random import uniform
import matplotlib.pyplot as plt
import imutils

IMAGES_DIR = "assets/images/test1"

CANVAS_WIDTH_MM = 620 # in millimeters
MAX_CANVAS_HEIGHT_MM = 1000#1000 

GA_PIXEL_DENSITY = 72 # 72 pixels per inch
PRINT_PIXEL_DENSITY = 900

CANVAS_WIDTH = int((CANVAS_WIDTH_MM / 25.4) * GA_PIXEL_DENSITY)
MAX_CANVAS_HEIGHT = int((MAX_CANVAS_HEIGHT_MM / 25.4) * GA_PIXEL_DENSITY)
MAX_ROTATION_DEG = 360

# get all image files in images directory
image_files = get_image_files(IMAGES_DIR)

# get contour 
contours = []

for image_file in image_files:
    img_path = f"{IMAGES_DIR}/{image_file}"
    img = cv2.imread(img_path, cv2.IMREAD_UNCHANGED)
    
    img_h, img_w, _ = img.shape

    contour = get_contour_from_image(img)

    scale = int((40 / 25.4) * GA_PIXEL_DENSITY) / img_w 

    contour = scale_contour(contour, scale)


    x,y,w,h = cv2.boundingRect(contour)

    contour = contour - [x, y]

    contours.append(contour)

TOTAL_POPULATION = 100#1000
# generate initial population
individuals = []
for i in range(TOTAL_POPULATION):
    pos_x = []
    pos_y = []
    angle = []

    for c in contours:
        left = tuple(c[c[:,:,0].argmin()][0])
        right = tuple(c[c[:,:,0].argmax()][0])
        top = tuple(c[c[:,:,1].argmin()][0])
        bottom = tuple(c[c[:,:,1].argmax()][0])

        w = right[0] - left[0]
        h = bottom[1] - top[1]

        side = h
        if w > h:
            side = w

        pos_x.append(uniform(0, CANVAS_WIDTH - side))
        pos_y.append(uniform(0, MAX_CANVAS_HEIGHT - side))
        angle.append(uniform(0, MAX_ROTATION_DEG))

    individuals.append(
        Individual(
            pos_x,
            pos_y,
            angle
        )
    )

def show_result(individual: Individual):
    PRINT_CANVAS_WIDTH = int((CANVAS_WIDTH / GA_PIXEL_DENSITY) * PRINT_PIXEL_DENSITY)
    PRINT_CANVAS_HEIGHT = int((MAX_CANVAS_HEIGHT / GA_PIXEL_DENSITY) * PRINT_PIXEL_DENSITY)

    canvas = np.zeros((PRINT_CANVAS_HEIGHT, PRINT_CANVAS_WIDTH, 4)).astype(np.uint8)

    pos_x = individual.get_pos_x()
    pos_y = individual.get_pos_y()
    angle = individual.get_angle()

    for i, image_file in enumerate(image_files):
        img_path = f"{IMAGES_DIR}/{image_file}"
        img = cv2.imread(img_path, cv2.IMREAD_UNCHANGED)

        _, ori_img_w, _ = img.shape

        scale  = int((10 / 25.4) * PRINT_PIXEL_DENSITY) / ori_img_w 

        img = cv2.resize(img, (0, 0), fx = scale, fy = scale)
        img = imutils.rotate_bound(img, angle[i])

        img_h, img_w, _ = img.shape

        x = int(pos_x[i] / GA_PIXEL_DENSITY * PRINT_PIXEL_DENSITY) 
        y = int(pos_y[i] / GA_PIXEL_DENSITY * PRINT_PIXEL_DENSITY)

        # if image exeeds most right side
        if (x + img_w) > PRINT_CANVAS_WIDTH:
            img_w = PRINT_CANVAS_WIDTH - x
        
        # if image exeeds most bottom side
        if (y + img_h) > PRINT_CANVAS_HEIGHT:
            img_h = PRINT_CANVAS_HEIGHT - y

        cv2.add(
            img[:img_h,:img_w,:], 
            canvas[y:y+img_h, x:x+img_w, :], 
            canvas[y:y+img_h, x:x+img_w, :],
            mask=img[:img_h,:img_w,3:]
        )

    non_zero = np.nonzero(canvas[:,:,3:])

    y0 = non_zero[0][0]
    y1 = non_zero[0][-1]

    canvas = canvas[y0:y1,:,:]

    cv2.imwrite('Test.png', canvas)
    cv2.imshow('Canvas', canvas)

def determine_fitness(individual: Individual):
    canvas = np.zeros((MAX_CANVAS_HEIGHT, CANVAS_WIDTH, 1)).astype(np.uint8)

    pos_x = individual.get_pos_x()
    pos_y = individual.get_pos_y()
    angle = individual.get_angle()

    intersection_score = 1 # initialy no intersection

    for i, contour in enumerate(contours):
        contour = contour + [int(pos_x[i]), int(pos_y[i])]
        contour = rotate_contour(contour, angle[i])

        # left = tuple(c[c[:,:,0].argmin()][0])
        right = tuple(c[c[:,:,0].argmax()][0])
        # top = tuple(c[c[:,:,1].argmin()][0])
        bottom = tuple(c[c[:,:,1].argmax()][0])

        # w = right[0]
        # x,y,w,h = cv2.boundingRect(contour)
        
        if (right[0] > CANVAS_WIDTH) or (bottom[1] > MAX_CANVAS_HEIGHT):
            # bad individual, eliminate it from population
            return 0
        
        # check if any intersection
        for i in range(contour.shape[0]):
            if canvas[contour[i][0][1],contour[i][0][0],0] != 0:
                intersection_score = 0
                break
        
        cv2.drawContours(canvas, [contour], -1, (255, 255, 255), cv2.FILLED)

    non_zero = np.nonzero(canvas)

    white_area = non_zero[0].size

    if white_area == 0:
        return 0

    y0 = non_zero[0][0]
    y1 = non_zero[0][-1]

    x0 = min(non_zero[1])
    x1 = max(non_zero[1])

    result_height = y1 - y0
    result_width = x1 - x0

    if result_height == 0 or result_width == 0:
        return 0
    
    result_area = result_width * result_height

    # black to area ratio
    bta_ratio = (result_area - white_area) / result_area 

    bta_ratio_score = 1 - bta_ratio

    final_score = intersection_score + bta_ratio_score

    return final_score

population = Population(individuals)

population.calculate_fitness(determine_fitness)

best_individuals = population.get_best_individuals(50)

best_fitnesses = []
gen_num = 0
while True:
    # generate new generation
    new_individuals = []

    for i in range(0, len(best_individuals), 2):
        # crossover (got 25 new individuals)
        child = best_individuals[i].crossover(best_individuals[i+1])

        # crossover (got 25 new individuals)
        mutated_child = child.mutate(CANVAS_WIDTH, MAX_CANVAS_HEIGHT, MAX_ROTATION_DEG)
        
        # mutation (got 50 new individuals)
        mutated_1 = best_individuals[i].mutate(CANVAS_WIDTH, MAX_CANVAS_HEIGHT, MAX_ROTATION_DEG)
        mutated_2 = best_individuals[i+1].mutate(CANVAS_WIDTH, MAX_CANVAS_HEIGHT, MAX_ROTATION_DEG)

        new_individuals.append(child)        
        new_individuals.append(mutated_child)
        new_individuals.append(mutated_1)
        new_individuals.append(mutated_2)

    population.update_individuals(new_individuals)

    population.calculate_fitness(determine_fitness)

    best_individuals = population.get_best_individuals(50)

    print(f"Generation {gen_num}\t: {best_individuals[0].get_fitness()}")

    best_fitnesses.append(best_individuals[0].get_fitness())

    if best_individuals[0].get_fitness() > 1.5:#22:
        break

    gen_num += 1

show_result(best_individuals[0])

print(f"Best\t: {best_individuals[0].get_fitness()}")

# plt.plot(range(len(best_fitnesses)), best_fitnesses)
# plt.show()

cv2.waitKey()
cv2.destroyAllWindows()   




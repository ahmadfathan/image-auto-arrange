from random import uniform

class Individual:
    def __init__(self, pos_x, pos_y, angle):
        self.__pos_x = pos_x
        self.__pos_y = pos_y
        self.__angle = angle

        self.__fitness = 0
    
    def set_fitness(self, fitness):
        self.__fitness = fitness

    def get_fitness(self):
        return self.__fitness
    
    def get_pos_x(self):
        return self.__pos_x
    
    def get_pos_y(self):
        return self.__pos_y
    
    def get_angle(self):
        return self.__angle
    
    def crossover(self, mate):
        return Individual(
            mate.get_pos_x(),
            self.__pos_y,
            mate.get_angle()
        )

    def mutate(self, max_x, max_y, max_angle):

        pos_x = [x + uniform(-(max_x - x),(max_x - x)) * 0.01 for x in self.__pos_x]
        pos_y = [y + uniform(-(max_y - y),(max_y - y)) * 0.01 for y in self.__pos_y]
        angle = [tetha + uniform(-(max_angle - tetha),(max_angle - tetha)) * 0.01 for tetha in self.__angle]

        return Individual(
            pos_x,
            pos_y,
            angle
        )
from threading import Thread

class Population:
    def __init__(self, individuals):
        self.__individuals = individuals

    def update_individuals(self, individuals):
        self.__individuals = individuals

    def __calculate_fitness(self, pos, individual, fitness_func):
        fitness = fitness_func(individual)
        self.__individuals[pos].set_fitness(fitness)
        
    def calculate_fitness(self, fitness_func):
        threads = []

        for i, individual in enumerate(self.__individuals):
            thread = Thread(target=self.__calculate_fitness, args=(i,individual,fitness_func,))

            threads.append(thread)

        for thread in threads: thread.start()
        for thread in threads: thread.join()
    
    def get_best_individuals(self, total):
        self.__individuals.sort(key=lambda x:x.get_fitness(), reverse=True)
        return self.__individuals[:total]
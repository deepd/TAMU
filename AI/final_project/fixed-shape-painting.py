import os, pygame; from pygame.locals import *
from decimal import Decimal
from PIL import Image, ImageChops
import random
import math
import operator
from time import sleep

height = 113
width = 118
ck = (0, 0, 0)
number_of_particles = 50
dataset = []
newfitness = 99999999999999
fitnessvalue = 0

class Particle:

	def __init__(self, (x, y), size, transparency, z, gs):
	    self.x = x
	    self.y = y
	    self.size = size
	    self.transparency = transparency
	    self.z = z
	    self.gs = gs

	def display(self):
		print "( %d, %d, %d, %d, %d )" % (self.x, self.y, self.size, self.transparency, self.z)

	def drawit(self):
		self.s = pygame.Surface((0,0))
		s = pygame.Surface(((int)(self.size*2), (int)(self.size*2)))
		self.s = s
		self.s.fill(ck)
		self.s.set_colorkey(ck)
  		pygame.draw.circle(self.s, (255, 255, 255), (self.size,self.size), self.size)
  		self.s.set_alpha(self.transparency)


def fitness(im):
	count = 0	
	target = Image.open("cd.jpg")
	target = target.load()
	i = 0
	while i < height:
	    j = 0
	    while j < width:
	        r = im[i,j][0] - target[i,j][0]
	        g = im[i,j][1] - target[i,j][1]
	        b = im[i,j][2] - target[i,j][2]
	        count = count + (r*r + g*g + b*b)
	        j = j+1
	    i = i+1
	return count


if __name__ == "__main__":
	for n in range(number_of_particles+1):
	    size = random.randint(0, width/6)
	    x = random.randint(2*size, width-(size*2))
	    y = random.randint(2*size, height-(size*2))
	    trans = random.randint(0, 255)
	    z = random.randint(0, number_of_particles)
	    gs = random.randint(0, 255)
	    dataset.append(Particle((x, y), size, trans, z, gs))

	for particle in dataset:
		particle.drawit()

	mainsurface = pygame.Surface((height,width))
	dataset = sorted(dataset, key=lambda data: data.z)
	for n in range(number_of_particles+1):
		mainsurface.blit(dataset[n].s, (dataset[n].x, dataset[n].y))

	pil_string_image = pygame.image.tostring(mainsurface, "RGB",False)
	pil_image = Image.fromstring("RGB",(height,width),pil_string_image)
	pil_image.save("genetic0.jpg")
	fitnessvalue = fitness(pil_image.load())
	print fitnessvalue

	for i in range(1,1000):
		newfitness = 9999999999999999
		while newfitness > fitnessvalue:
			for k in range(1):
				mutation_param = random.randint(1,6)
				j = random.randint(0,number_of_particles - 1)
				if mutation_param == 1:
					dataset[j].size = random.randint(0, width/6)
				if mutation_param == 2:
					dataset[j].x = random.randint(2*dataset[j].size, width-2*dataset[j].size)
				if mutation_param == 3:
					dataset[j].y = random.randint(2*dataset[j].size, height-2*dataset[j].size)
				if mutation_param == 4:
					dataset[j].transparency = random.randint(0, 255)
				if mutation_param == 5:
					dataset[j].z = random.randint(0, number_of_particles)
				if mutation_param == 6:
					dataset[j].gs = random.randint(0, 255)
					dataset[j].drawit()
			mainsurface2 = pygame.Surface((height,width))
			dataset = sorted(dataset, key=lambda data: data.z)
			for n in range(number_of_particles+1):
				mainsurface2.blit(dataset[n].s, (dataset[n].x, dataset[n].y))
			pil_string_image2 = pygame.image.tostring(mainsurface2, "RGB",False)
			pil_image2 = Image.fromstring("RGB",(height,width),pil_string_image2)
			newfitness = fitness(pil_image2.load())
			if newfitness < fitnessvalue:
				print newfitness
				print "success"
				pil_image2.save("genetic%s.jpg" % i)
				fitnessvalue = newfitness




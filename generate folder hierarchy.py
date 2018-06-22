import os

def createHierarchy(root, depth, branching_factor, no_of_files):
	
	if depth == 1:
		#create files
		file_data = "Hello, how are you!!"

		for i in range(no_of_files):
			name = root + '/' + str(i+1)+'txt'
			fp = open(name,"w")
			fp.write(file_data)
			fp.close()

	else:
		#create more folders
		for i in range(branching_factor):
			name = root + str(i+1) + '/'
			os.mkdir(name)
			#chdir(name)
			createHierarchy(name, depth-1, branching_factor, no_of_files)
			#chdir(name)



if __name__ == '__main__':

	root = 'Hello/'
	os.mkdir(root)

	branching_factor = 4
	depth = 8
	no_of_files = 3

	createHierarchy(root, depth, branching_factor, no_of_files)


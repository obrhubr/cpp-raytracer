# Get the base Ubuntu image from Docker Hub
FROM gcc:latest

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install the Clang compiler
RUN apt-get -y install gcc

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /usr/src/raytracer

# Specify the working directory
WORKDIR /usr/src/raytracer

# Use Clang to compile the Test.cpp source file
RUN make

# Run the output program from the previous step
CMD ["./raytracer.exe"]
# GdalMap

GdalMap est un développement en C++ qui a pour objet de mettre en oeuvre GDAL avec une interface codée grâce à JUCE (www.juce.com).
Ce développement devrait donc être portable sur Windows, Linux et MacOS.
L'intérêt de ce développement est purement pédagogique. 

GdalMap is a software written in C++ that uses GDAL (gdal.org) with a user interface developed with JUCE (www.juce.com).
The code should compile on Windows, Linux and MacOS.
The main purpose is educational.

## Installation
Pour installer GdalMap, il faut installer JUCE (www.juce.com) et GDAL.
Pour GDAL, le plus simple est d'utiliser Anaconda (https://anaconda.org/) et dans une console, il suffit de faire :

`conda create --name gdal`

`conda activate gdal`

`conda install -c conda-forge gdal`

(On crée un environnement gdal que l'on active, puis on installe GDAL).
Le projet doit être modifié avec le Projucer de JUCE.


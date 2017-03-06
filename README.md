# Candy Crush

## Current Progress:

Just finished implementation of TCP to seperate the model and view parts of this application across a network. As the server, the view is responsible for passing the initial game instance to the model and updating the window. As the client, the model  is responsible for firing candies, inflicting gravity, and passing the updated game instance back to the server.

/array2d contains the source code for our own 2D array library

/centralized contains the version of this game created before TCP implementation and seperation of the model and view

/distributed contains the most up-to-date version of this game
</br>

[Click here for project description](http://courses.cs.washington.edu/courses/cse333/17wi/hw/hw5/hw5.html)

## Copyright

2017 Nolan Strait, Linxing Preston Jiang

multi-body lambda expressions have not been fully implemented in this project.
That is, something like (lambda (bool) (set! bool (not bool)) bool) will not eval.
I believe this is a big problem to solve given the assumptions made on previous
portions of the project. Since the bodies might have mutual dependencies on both
the paramNames and each other they would have to be have their bindings go into
the global topFrame. So in my examplebody1 = (set! state (not state)) needs to
alter the frame which body2 = state uses. This happens nowhere else in the
project other than at the highest level of the tree when void interpret(tree) is
first called.

Optional:
Implemented >=, <=, and eq?
Also did seperate cases for adding int+int, int+doub, and doub+doub

Enjoy your summer break!

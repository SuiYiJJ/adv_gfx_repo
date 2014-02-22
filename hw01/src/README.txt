HOMEWORK 1: SIMPLIFICATION & SUBDIVISION

NAME:  < Make Espinoza >


TOTAL TIME SPENT:  <  30+ >
Please estimate the number of hours you spent on this assignment.


COLLABORATORS: 
You must do this assignment on your own, as described in the 
Academic Integrity Policy.  If you did discuss the problem or errors
messages, etc. with anyone, please list their names here.
  * Just Lectures
  * http://graphics.stanford.edu/~mdfisher/subdivision.html
  * Josh's 5 minute crash course in gdb

GOURAUD SHADING EFFICIENCY/PERFORMANCE:
  * Made it gather the average around a vertex, taking both directions 
    (clockwise/counter clockwise around a vertex) into consideration
  * Only computationally expensive part is gathering normals around a vertex,
      considering most vertex have valence of 6, it didn't add to overall bounding time.
  * Bounded by order of O(v^2), where v are the vertices's of our graph. 
    Collecting adjacent vertices's could be done in constant about 6 per vertex time.

SIMPLIFICATION/EDGE COLLAPSE NOTES:
  * Made it robust, by taking into consideration the manifold problem.
  * I have an ignore list, and pick the shortest possible edge and check if it doesn't break manifold or is already in the ignore list.
  * More expensive, but still bound in poly-time. 

SUBDIVISION NOTES:
    * Had a lot of fun doing this. Implemented loop's subdivision.
    * Also did infinite creases described in Hoppe's paper along with his other vertex/edge mask.


KNOWN BUGS IN YOUR CODE:
Other then sometimes simplification breaking a 2D object or rending slight blue spots not much.
Subdivision is slow past 4 or 5 attempts! Also sorry for all the debug prints to console :(

NEW FEATURES OR EXTENSIONS FOR EXTRA CREDIT:
Really robust, runs on all test cases! I spent a lot of time making it not crash on some of the inputs. (Probably not extra credit though)


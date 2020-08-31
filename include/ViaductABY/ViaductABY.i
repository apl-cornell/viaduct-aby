 %module ViaductABY
 %{
 /* Includes the header in the wrapper code */
 #include <ViaductABY/ViaductABY.h>
 %}
 
 /* Parse the header file to generate wrappers */
 %include "ViaductABY.h"
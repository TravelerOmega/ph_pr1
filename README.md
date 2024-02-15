# ph_pr1
Práctica 1 de la asignatura de Programación Hardware.

Se ha implementado un driver que imita al portapapeles clásico, con la diferencia de que posee tres buffers en los que almacenar información.
Se puede escribir sobre cada uno de los buffers manualmente, y posteriormente leer el contenido de cada uno por separado.
Adicionalmente, si se escribe en un buffer determinado, el mensaje que previamente hubiera en él se mueve al siguiente buffer de forma consecutiva
(el contenido del buffer 0 pasaría al 1, el del 1 al 2, y el del 2 se perdería). Esto permite tener un historial de elementos almacenados que se
actualiza automáticamente.

El tamaño de cada buffer es configurable y se puede definir a la hora de llamar al driver mediante los parámetros size0, size1 y size2.

Se puede escribir a un buffer usando echo, leer con cat, y se puede desplazar el contenido de cualquier buffer a disco duro con un simple pipeline empleando cat.

El tamaño por defecto de los buffer es 255 bytes, que se corresponde a su vez con el máximo. Intentar poner mayor tamaño para los buffer (o un tamaño negativo) causará un error.

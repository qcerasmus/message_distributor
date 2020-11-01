# message_distributor
This project was made to be an alternative to the likes of rabbitmq, zeromq and so on. 
The main selling point is easy integration as it's a header only library.

It is still very much a work in progress with lots of things I would like to add.


Dependencies
------------
* asio: https://github.com/chriskohlhoff/asio
* cereal: https://github.com/USCiLab/cereal.git
* concurrentqueue: https://github.com/cameron314/concurrentqueue.git
* spdlog: https://github.com/gabime/spdlog.git

All of the dependencies will be downloaded on the first build of the project.
They will be saved to the 'third_party' folder each in their own folder.


Usage
-----
To use this library you have to use 

cmake: https://cmake.org/
or an IDE that supports CMakeLists.txt files out the box.
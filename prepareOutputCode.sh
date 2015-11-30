echo "" > output.cpp
cat inc/GameController.hpp > output.cpp
cat src/GameController.cpp | grep -v "#include" >> output.cpp
cat src/main.cpp | grep -v "#include" >> output.cpp
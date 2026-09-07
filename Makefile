CXX = clang++

CXXFLAGS = -std=c++20 \
           -I/opt/homebrew/include/opencascade

SRC = src/makeNozzle.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = run

LDFLAGS = -L/opt/homebrew/lib

LDLIBS = \
    -lTKernel \
    -lTKMath \
    -lTKG2d \
    -lTKG3d \
    -lTKGeomBase \
    -lTKGeomAlgo \
    -lTKBRep \
    -lTKTopAlgo \
    -lTKPrim \
    -lTKXSBase \
    -lTKDE \
    -lTKDESTEP

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean rebuild

rebuild: clean all

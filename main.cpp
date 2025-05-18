#include <raylib.h>
#include <iostream>
#include <math.h>
#include <random>
#define GRID_WIDTH 128
#define GRID_HEIGHT 128

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define FRAME_RATE 60

float randomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

enum P_Types {
    NONE,
    SAND,
    SOLID,
    WATER
};

struct Particle {
    signed int direction = 0;
    Color color = BLACK;
    P_Types type = P_Types::NONE;
};

struct None_Particle : Particle {
    None_Particle() {
        this->color = BLACK;
        this->type = P_Types::NONE;
    }
};

struct Sand_Particle : Particle {
    Sand_Particle() {
        this->color = ColorFromHSV(randomFloat(50,60), randomFloat(0.8,1), randomFloat(0.7,0.9)); 
        this->type = P_Types::SAND;
    }
};

struct Rock_Particle : Particle {
    Rock_Particle() {
        this->color = ColorFromHSV(0, 0, randomFloat(0.6,0.8));
        this->type = P_Types::SOLID;
    }
};

struct Water_Particle : Particle {
    Water_Particle(signed int direction = 0) {
        this->color = ColorFromHSV(randomFloat(220,240), randomFloat(0.8,1), randomFloat(0.7,0.9));
        this->type = P_Types::WATER;

        this->direction = direction;
    }
};


class Grid {
    public:
        Grid() {

        };

        Particle getParticle(int x, int y) {
            return this->particles[y][x];
        }

        Vector2 getGridByMouse() {        
            return {
                round(GetMouseX() / viewportSizes[0]),
                round(GetMouseY() / viewportSizes[1])
            };
        }

        void setParticle(int x, int y, Particle newParticle) {
            this->particles[y][x].color = newParticle.color;
            this->particles[y][x].type = newParticle.type;
        }

        void removeParticle(int x,int y) {
            setParticle(x,y,None_Particle());
        }

        void drawGrid() {
            for(int i=0;i<GRID_HEIGHT-1;i++) {
                for(int j=0;j<GRID_WIDTH-1;j++) {
                    const int y = GRID_HEIGHT - i;
                    const int x = (j % 2 == 0) ? GRID_WIDTH - j : j;
                    const P_Types typeOfParticle = this->particles[y][x].type;
                    if (typeOfParticle == P_Types::NONE || y>=GRID_HEIGHT) {
                        continue;
                    }
                    DrawRectangle(rect_size * x,rect_size*y,rect_size,rect_size, this->particles[y][x].color);
                    if(typeOfParticle == P_Types::SAND){
                        handleSandGravity(x,y);
                    } else if(typeOfParticle == P_Types::WATER) {
                        handleWaterGravity(x,y);
                    }
                }
            }
        }
    private:
        Particle (particles)[GRID_HEIGHT][GRID_WIDTH];
        const float viewportSizes[2] = {
            floor(WINDOW_WIDTH/GRID_WIDTH),
            floor(WINDOW_HEIGHT/GRID_HEIGHT)
        };
        const int rect_size = floor(WINDOW_HEIGHT / GRID_HEIGHT);
    


        bool isEmptyUnder(int x, int y) {
            if(y+2 > GRID_HEIGHT) {
                return false;
            }
            if(this->particles[y+1][x].type == P_Types::NONE) {
                return true;
            }
            return false;
        }

        bool isEmptyOnLeft(int x, int y) {
            if(x-1 <= 0) {
                return false;
            }
            if(this->particles[y][x-1].type == P_Types::NONE) {
                return true;
            }
            return false;
        }
        Color getColor(int x, int y) {
            return particles[y][x].color;
        } 
        bool isEmptyOnRight(int x, int y) {
            if(x+1 >= GRID_WIDTH) {
                return false;
            }
            if(this->particles[y][x+1].type == P_Types::NONE) {
                return true;
            }
            return false;
        }
        void handleSandGravity(int x, int y) {
            if (isEmptyUnder(x,y)) {
                removeParticle(x,y);
                setParticle(x,y+1,Sand_Particle());
                return;
            }
            
            if (rand() % 2 == 0) {
                if (isEmptyOnRight(x,y) && isEmptyUnder(x+1,y)) {
                    removeParticle(x,y);
                    setParticle(x+1,y+1,Sand_Particle());
                }
            } else {
                if (isEmptyOnLeft(x,y) && isEmptyUnder(x-1,y)) {
                    removeParticle(x,y);
                    setParticle(x-1,y+1,Sand_Particle());
                }
            }
        }

        void handleWaterGravity(int x, int y) {
            signed int *direction = &particles[y][x].direction;
            const Water_Particle myWaterParticle = Water_Particle(*direction);

            if(*direction == 0) {
                *direction = rand() % 2 == 0 ? -1 : 1;
            }


            if(isEmptyUnder(x,y)) {
                removeParticle(x,y);
                setParticle(x,y+1,myWaterParticle);
                return;
            }

            int wherePlace = 0;
            if(rand() % 2 == 0) {
                if(isEmptyOnRight(x,y)) {
                    wherePlace = 1;
                } else if(isEmptyOnLeft(x,y)) {
                    wherePlace = -1;
                } 
            } else {
                if(isEmptyOnLeft(x,y)) {
                    wherePlace = -1;
                } else if (isEmptyOnRight(x,y)) {
                    wherePlace = 1;
                }
            }

            if(wherePlace != 0) {
                removeParticle(x,y);
                setParticle(x+wherePlace,y,myWaterParticle);
            }
        }
};

void drawParticles(int x,int y,Grid* grid, Particle* selectedType) {
    for(int i=-1;i<1;i++) {
        for(int j=-1;j<1;j++) {
            grid->setParticle(x+i, y+j, *selectedType);
        }
    }
}

void handleInputs(Grid* grid, Particle* selectedType) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        drawParticles(grid->getGridByMouse().x,grid->getGridByMouse().y,grid,selectedType);
    } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        grid->removeParticle(grid->getGridByMouse().x, grid->getGridByMouse().y);
    }

    if (IsKeyDown(KEY_Q)) {
        *selectedType = Sand_Particle();
    }
    if (IsKeyDown(KEY_W)) {
        *selectedType = Rock_Particle();
    }
    if (IsKeyDown(KEY_E)) {
        *selectedType = Water_Particle();
    }
}

int main() {
    Grid grid {};
    grid.setParticle(8,4,Sand_Particle());
    grid.setParticle(8,2,Sand_Particle());
    grid.setParticle(16,15,Sand_Particle());
    grid.setParticle(17,15,Sand_Particle());

    Particle selectedType = Sand_Particle();

    InitWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"sand falling simulator");
    SetTargetFPS(FRAME_RATE);
    while(!WindowShouldClose()) {
        handleInputs(&grid, &selectedType);


        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawText(TextFormat("Selected: %i", selectedType.type),5,5,16,WHITE);
        grid.drawGrid();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <ctime>
#include <cmath>

struct Position
{
    Position();
    Position(int x, int y);
    int x_;
    int y_;
};

struct Human
{
    Human();
    Human(int id, Position pos);
    bool operator<(const Human& rhs) const;
    int id_;
    Position pos_;
    enum Category
    {
        OK,
        Endangered,
        Lost
    };
    mutable Category cat_;
};

struct Zombie
{
    Zombie();
    Zombie(int id, Position pos, Position nextPos);
    bool operator<(const Zombie& rhs) const;
    int id_;
    Position pos_;
    Position nextPos_;
    mutable double appealFactor_;
};

struct GameData
{
    Position ashPos_;
    int humanCount_;
    std::set<Human> humans_;
    int zombieCount_;
    std::set<Zombie> zombies_;
};

namespace Helpers
{
const int ashStepSize = 1000;
const int zombieStepSize = 400;
const int shootingRadius = 2000;
const double zombieFactor = 1;
const double humanFactor = 1.0;
const double endangeredFactor = 40.0;
const double ashFactor = 40.0;
const double neighbourhoodRadius = 3000;
double distance(Position p1, Position p2);
double distance(Position ash, Human human);
double distance(Position ash, Zombie zombie);
int steps(Human, Zombie);
int steps(Human, Position);
int steps(Position, Zombie);
}

namespace VectorOpers
{
    Position subtract(Position p2, Position p1);
    Position multiply(Position pos, double n);
    Position resize(Position pos, double length);
    Position rotate(Position pos, double angle);
}


class GameController
{
public:
    GameController();
    ~GameController();
    void startGame();
    void loadGameData(std::istream& input);
    void writeSolution(Position sol);

    void chooseStrategy();
    void doTheTriage();
    void doTheTriage(std::set<Human> &humans);
    bool atLeastOneHumanIsSave();
    bool atLeastOneHumanIsSave(std::set<Human> const &humans);
    void rateZombies();
    void rateZombies(std::set<Zombie> &zombies);

    Position dumbStrategy();
    Position rescueMissionStrategy();
    Position goToClosestEndangered();
    Position attackMostDenseZombie();

    Zombie findNearestZombie(Position pos, std::set<Zombie> const &zombies);
    Zombie findZombieWithHighestAppealFactor(std::set<Zombie> const &zombies);
    std::vector<Zombie> selectZombiesFromNeighbourhood(
        Zombie zombie, std::set<Zombie> const &zombies);
    Position centerOfMass(std::vector<Position> const &positions);
    Position centerOfMass(std::vector<Zombie> const &zombies);
    Position calcDestination(Position start, Position vec);
    int countZombiesInRange(Position pos, std::set<Zombie> const &zombies);
    double chooseBestAngle(Position vec, std::set<Zombie> const &zombies);

    GameData getData();
    void debugPrint(GameData data);
private:
    enum State
    {
        normalMode,
        rescueEndangered,
        rescueHuman
    };
    GameData data_;
    State state_;
};
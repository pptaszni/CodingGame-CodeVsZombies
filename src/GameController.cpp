#include "GameController.hpp"

#define DEBUG_PRINT 1
#define MAX_DIST 20000.0

using namespace std;

Position::Position(): Position(0,0)
{}

Position::Position(int x, int y): x_(x), y_(y)
{}

Human::Human(): Human(0, Position())
{}

Human::Human(int id, Position pos): id_(id), pos_(pos)
{
    cat_ = OK;
}

bool Human::operator<(const Human& rhs) const
{
    return id_ < rhs.id_;
}

Zombie::Zombie(): Zombie(0,Position(),Position())
{}

Zombie::Zombie(int id, Position pos, Position nextPos):
    id_(id), pos_(pos), nextPos_(nextPos)
{
    appealFactor_ = -1;
}

bool Zombie::operator<(const Zombie& rhs) const
{
    return id_ < rhs.id_;
}

double Helpers::distance(Position p1, Position p2)
{
    double x = p2.x_ - p1.x_;
    double y = p2.y_ - p1.y_;
    return sqrt(x*x + y*y);
}

double Helpers::distance(Position ash, Human human)
{
    return distance(ash, human.pos_);
}

double Helpers::distance(Position ash, Zombie zombie)
{
    return distance(ash, zombie.nextPos_);
}

int Helpers::steps(Human human, Zombie zombie)
{
    double dist = Helpers::distance(
        human.pos_, zombie);
    dist = dist/Helpers::zombieStepSize;
    return ceil(dist);
}

int Helpers::steps(Human human, Position pos)
{
    double dist = Helpers::distance(
        pos, human);
    dist = dist - Helpers::shootingRadius;
    dist = dist/Helpers::ashStepSize;
    return ceil(dist);
}

int Helpers::steps(Position pos, Zombie zombie)
{
    double dist = Helpers::distance(
        pos, zombie);
    dist = dist - Helpers::shootingRadius;
    dist = dist/Helpers::ashStepSize;
    return ceil(dist);
}

Position VectorOpers::subtract(Position p2, Position p1)
{
    return Position(p2.x_ - p1.x_, p2.y_ - p1.y_);
}

Position VectorOpers::multiply(Position pos, double n)
{
    return Position(pos.x_*n, pos.y_*n);
}

Position VectorOpers::resize(Position pos, double length)
{
    double vecLen = Helpers::distance(
        Position(0,0), pos);
    double x = pos.x_;
    double y = pos.y_;
    x = x*length;
    y = y*length;
    x = x/vecLen;
    y = y/vecLen;
    return Position(x,y);
}

Position VectorOpers::rotate(Position pos, double angle)
{
    double x = cos(angle)*pos.x_ - sin(angle)*pos.y_;
    double y = sin(angle)*pos.x_ + cos(angle)*pos.y_;
    return Position(x,y);
}

GameController::GameController()
{
    state_ = normalMode;
}

GameController::~GameController()
{}

void GameController::startGame()
{
    Position solution;
    while (1)
    {
        loadGameData(std::cin);
        chooseStrategy();
        switch (state_)
        {
            case normalMode:
            {
                solution = attackMostDenseZombie();
                break;
            }
            case rescueEndangered:
            {
                solution = goToClosestEndangered();
                break;
            }
            case rescueHuman:
            {
                solution = rescueMissionStrategy();
                break;
            }
        }
        if (DEBUG_PRINT)
        {
            debugPrint(data_);
            cerr << "State: " << state_ << endl;
        }
        writeSolution(solution);
    }
}

void GameController::loadGameData(std::istream& input)
{
    data_.humans_.clear();
    data_.zombies_.clear();
    int x,y;
    input >> x >> y; input.ignore();
    Position ashPos(x,y);
    data_.ashPos_ = ashPos;
    input >> data_.humanCount_; input.ignore();
    for (int i = 0; i < data_.humanCount_; i++)
    {
        int humanId, humanX, humanY;
        input >> humanId >> humanX >> humanY; input.ignore();
        Human hum(humanId, Position(humanX, humanY));
        data_.humans_.insert(hum);
    }
    input >> data_.zombieCount_; input.ignore();
    for (int i = 0; i < data_.zombieCount_; i++)
    {
        int zombieId, zombieX, zombieY, zombieXNext, zombieYNext;
        input >> zombieId >> zombieX >> zombieY >> zombieXNext >> zombieYNext; input.ignore();
        Zombie zom(zombieId,
            Position(zombieX, zombieY),
            Position(zombieXNext, zombieYNext));
        data_.zombies_.insert(zom);
    }
}

void GameController::writeSolution(Position sol)
{
    cout << sol.x_ << " " << sol.y_ << endl;
}

void GameController::chooseStrategy()
{
    if (data_.humanCount_ == 1)
    {
        state_ = rescueHuman;
        return;
    }
    doTheTriage();
    if (atLeastOneHumanIsSave())
    {
        rateZombies();
        state_ = normalMode;
    }
    else
    {
        state_ = rescueEndangered;
    }
}
void GameController::doTheTriage()
{
    doTheTriage(data_.humans_);
}

void GameController::doTheTriage(set<Human> &humans)
{
    for (auto &human: humans)
    {
        Zombie nearestZombie = findNearestZombie(
            human.pos_, data_.zombies_);
        int zombieSteps = Helpers::steps(human,
            nearestZombie);
        if (zombieSteps <= 0)
        {
            human.cat_ = Human::Category::Lost;
            continue;
        }
        int ashSteps = Helpers::steps(human,
            data_.ashPos_);
        int diffSteps = zombieSteps - ashSteps;
        if (diffSteps <= 0)
        {
            human.cat_ = Human::Category::Lost;
        }
        else if (diffSteps <= 2)
        {
            human.cat_ = Human::Category::Endangered;
        }
    }
}

bool GameController::atLeastOneHumanIsSave()
{
    return atLeastOneHumanIsSave(data_.humans_);
}

bool GameController::atLeastOneHumanIsSave(set<Human> const &humans)
{
    for (auto human: humans)
    {
        if (human.cat_ == Human::Category::OK)
        {
            return true;
        }
    }
    return false;
}

void GameController::rateZombies()
{
    rateZombies(data_.zombies_);
}

void GameController::rateZombies(set<Zombie> &zombies)
{
    for (auto &zombie: zombies)
    {
        double totalFactor = 0;
        double safeFactor = 400*400; // no division by zero
        Position zombPos = zombie.nextPos_;
        for (auto zom: zombies)
        {
            if (zom.id_ == zombie.id_)
                continue;
            double distSqr = pow(
                Helpers::distance(zombPos, zom),2);
            distSqr += safeFactor;
            totalFactor += Helpers::zombieFactor/distSqr;
        }
        for (auto hum: data_.humans_)
        {
            if (hum.cat_ == Human::Category::OK)
            {
            double distSqr = pow(
                Helpers::distance(zombPos, hum),2);
            distSqr += safeFactor;
            totalFactor += Helpers::humanFactor/distSqr;
            }
            else if (hum.cat_ == Human::Category::Endangered)
            {
            double distSqr = pow(
                Helpers::distance(zombPos, hum),2);
            distSqr += safeFactor;
            totalFactor += Helpers::endangeredFactor/distSqr;
            }
        }
        double distSqr = pow(
            Helpers::distance(data_.ashPos_, zombPos),2);
        distSqr += safeFactor;
        totalFactor += Helpers::ashFactor/distSqr;
        zombie.appealFactor_ = totalFactor;
    }
}

Position GameController::dumbStrategy()
{
    Zombie nearestZombie = findNearestZombie(
        data_.ashPos_, data_.zombies_);
    return Position(nearestZombie.nextPos_);
}

Position GameController::rescueMissionStrategy()
{
    Position lastHumanPosition(
        data_.humans_.begin()->pos_);
    if (data_.ashPos_.x_ == lastHumanPosition.x_
        && data_.ashPos_.y_ == lastHumanPosition.y_)
    {
        Zombie nearest = findNearestZombie(
            data_.ashPos_, data_.zombies_);
        Position zombieCenter = centerOfMass(
            selectZombiesFromNeighbourhood(
                nearest, data_.zombies_));
        return zombieCenter;
    }
    else
    {
        return lastHumanPosition;
    }
}

Position GameController::goToClosestEndangered()
{
    Position target;
    for (auto human: data_.humans_)
    {
        if (human.cat_ == Human::Category::Endangered)
        {
            target = human.pos_;
            break;
        }
    }
    return target;
}

Position GameController::attackMostDenseZombie()
{
    Zombie bestZombie = findZombieWithHighestAppealFactor(
        data_.zombies_);
    Position zombieCenter = centerOfMass(
            selectZombiesFromNeighbourhood(
                bestZombie, data_.zombies_));
    Position vec = VectorOpers::subtract(
        zombieCenter, data_.ashPos_);
    vec = VectorOpers::resize(vec, Helpers::ashStepSize);
    double bestAngle = chooseBestAngle(vec, data_.zombies_);
    vec = VectorOpers::rotate(vec, bestAngle);
    Position calculatedPos = calcDestination(
        data_.ashPos_, vec);
    if (DEBUG_PRINT)
    {
        cerr << "bestAngle: " << bestAngle << endl;
    }
    return calculatedPos;
}

Zombie GameController::findNearestZombie(
    Position pos, set<Zombie> const &zombies)
{
    double minDist = MAX_DIST;
    Zombie nearestZombie;

    for (auto zomb: zombies)
    {
        double dist = Helpers::distance(pos, zomb);
        if (dist < minDist)
        {
            minDist = dist;
            nearestZombie = zomb;
        }
    }
    return nearestZombie;
}

Zombie GameController::findZombieWithHighestAppealFactor(
    set<Zombie> const & zombies)
{
    double maxAppeal = 0;
    Zombie bestZombie;
    for (auto zombie: zombies)
    {
        if (zombie.appealFactor_ > maxAppeal)
        {
            maxAppeal = zombie.appealFactor_;
            bestZombie = zombie;
        }
    }
    return bestZombie;
}

vector<Zombie> GameController::selectZombiesFromNeighbourhood(
    Zombie refZombie, set<Zombie> const &zombies)
{
    vector<Zombie> neighbours;
    Position refPos = refZombie.nextPos_;
    for (auto zombie: zombies)
    {
        double dist = Helpers::distance(refPos, zombie);
        if (dist < Helpers::neighbourhoodRadius)
        {
            neighbours.push_back(zombie);
        }
    }
    return neighbours;
}

Position GameController::centerOfMass(vector<Position> const &positions)
{
    if (positions.size() == 0)
        return Position(0,0);
    int xCenter = 0;
    int yCenter = 0;
    for (auto position: positions)
    {
        xCenter += position.x_;
        yCenter += position.y_;
    }
    xCenter = xCenter/positions.size();
    yCenter = yCenter/positions.size();
    return Position(xCenter, yCenter);
}

Position GameController::centerOfMass(vector<Zombie> const &zombies)
{
    vector<Position> positions;
    for (auto zombie: zombies)
    {
        positions.push_back(zombie.nextPos_);
    }
    return centerOfMass(positions);
}

Position GameController::calcDestination(
    Position start, Position vec)
{
    return Position(start.x_+vec.x_,
        start.y_+vec.y_);
}

int GameController::countZombiesInRange(
    Position pos, set<Zombie> const &zombies)
{
    int numZombies = 0;
    for (auto zombie: zombies)
    {
        double dist = Helpers::distance(
            pos, zombie);
        if (dist <= Helpers::shootingRadius)
        {
            numZombies++;
        }
    }
    return numZombies;
}

double GameController::chooseBestAngle(
    Position vec, set<Zombie> const &zombies)
{
    int bestZombiesCount = countZombiesInRange(
        data_.ashPos_, data_.zombies_);
    if (bestZombiesCount == 0)
        return 0.0;
    double bestAngle = 0.0;
    for (double angle=-0.5; angle<=0.5; angle=angle+0.1)
    {
        Position newVec = VectorOpers::rotate(vec,angle);
        Position dest = calcDestination(data_.ashPos_, newVec);
        int zomCount = countZombiesInRange(dest, data_.zombies_);
        if (zomCount > bestZombiesCount)
        {
            bestZombiesCount = zomCount;
            bestAngle = angle;
        }
    }
    return bestAngle;
}

GameData GameController::getData()
{
    return data_;
}

void GameController::debugPrint(GameData data)
{
    cerr << "AshPos: " << data.ashPos_.x_
        << " " << data.ashPos_.y_ << endl;
    cerr << "HumanCount: " << data.humanCount_ << endl;
    for (auto human: data.humans_)
    {
        cerr << "[" << human.id_ << "] "
            << human.pos_.x_ << " "
            << human.pos_.y_ << ", cat: "
            << human.cat_ << endl;
    }
    cerr << "ZombieCount: " << data.zombieCount_ << endl;
    for (auto zombie: data.zombies_)
    {
        cerr << "[" << zombie.id_ << "] "
            << zombie.pos_.x_ << " "
            << zombie.pos_.y_ << ", "
            << zombie.nextPos_.x_ << " "
            << zombie.nextPos_.y_ << ", appeal: "
            << zombie.appealFactor_ << endl;
    }
}
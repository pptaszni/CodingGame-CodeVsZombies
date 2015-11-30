#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>
#include <cmath>

#include "GameController.hpp"

using namespace std;


TEST(HelperFunctionsTest, distanceShouldReurnEuclideanNorm)
{
    Position ash(8000, 4500);
    Human human(0, Position(1000, 2000));
    Zombie zombie(0, Position(300, 400), Position(3000, 4000));
    double err = 1.0e-3;
    double dist1, dist2;

    dist1 = Helpers::distance(ash, human);
    dist2 = Helpers::distance(ash, zombie);

    ASSERT_LE(abs(7433.03437366-dist1), err);
    ASSERT_LE(abs(5024.93781056-dist2), err);
}

TEST(HelperFunctionsTest, stepShouldReturnNumberOfRoundsToReachTarget)
{
    Position ash(8000, 4500);
    Human human(0, Position(3000, 4500));
    Zombie zombie(0,Position(2500,4500), Position(2900, 4500));

    ASSERT_EQ(1, Helpers::steps(human, zombie));
    ASSERT_EQ(3, Helpers::steps(human, ash));
    ASSERT_EQ(4, Helpers::steps(ash, zombie));
    zombie.nextPos_.x_ = 3000;
    ASSERT_EQ(0, Helpers::steps(human, zombie));
    ASSERT_EQ(3, Helpers::steps(ash, zombie));
}

TEST(VectorOpersTest, multiplicationTest)
{
    Position v1(12,13);
    Position v2;
    double n = 10.0;
    v2 = VectorOpers::multiply(v1,n);
    ASSERT_EQ(n*v1.x_, v2.x_);
    ASSERT_EQ(n*v1.y_, v2.y_);
}

TEST(VectorOpersTest, resizedVectorShouldHaveTheSameAngleAndNewLength)
{
    int x = 12335;
    int y = -543166;
    double newLength = 1000;
    int err = 1;
    Position v1(x,y);
    v1 = VectorOpers::resize(v1, newLength);
    if (v1.x_ != 0)
    {
        ASSERT_LE(abs(y/x - v1.y_/v1.x_), err);
    }
    else
    {
        ASSERT_LE(abs(newLength-v1.y_), err);
    }
    double length = Helpers::distance(Position(), v1);
    ASSERT_LE(abs(newLength - length), err);
}

TEST(VectorOpersTest, basicVectorsAreRotatedCorrectly)
{
    Position v1(1000,0);
    double pi = 3.14159265;
    v1 = VectorOpers::rotate(v1, pi/2);
    ASSERT_EQ(0, v1.x_);
    ASSERT_EQ(1000, v1.y_);
}

class GameControllerShould: public testing::Test
{
public:
    GameController sut_;
};

TEST_F(GameControllerShould, returnCorrectDataAfterLoadingItTwice)
{
    GameData dat;
    ifstream ifs;

    ifs.open(
        "data/sampleRoundData.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    ifs.open(
        "data/sampleRoundData.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    dat = sut_.getData();

    ASSERT_EQ(2, dat.humanCount_);
    ASSERT_EQ(4, dat.zombieCount_);
    ASSERT_EQ(dat.humanCount_, dat.humans_.size());
    ASSERT_EQ(dat.zombieCount_, dat.zombies_.size());
}

TEST_F(GameControllerShould, returnCorrectDataAfterHumanAndZombieDeath)
{
    GameData dat1, dat2;
    ifstream ifs;

    ifs.open(
        "data/sampleRoundData.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    dat1 = sut_.getData();
    ifs.open(
        "data/sampleDataHumanZombieDead.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    dat2 = sut_.getData();

    ASSERT_EQ(dat1.humans_.size()-1, dat2.humans_.size());
    ASSERT_EQ(dat1.zombies_.size()-1, dat2.zombies_.size());
    for (auto human: dat2.humans_)
    {
        Human oldHum = *(dat1.humans_.find(human));
        ASSERT_EQ(oldHum.pos_.x_, human.pos_.x_);
        ASSERT_EQ(oldHum.pos_.y_, human.pos_.y_);
    }
    for (auto zombie: dat2.zombies_)
    {
        Zombie oldZom = *(dat1.zombies_.find(zombie));
        ASSERT_EQ(oldZom.pos_.x_+10, zombie.pos_.x_);
    }
}

TEST_F(GameControllerShould, findNearestZombieInTheSet)
{
    GameData dat;
    ifstream ifs;
    Zombie foundZombie;

    ifs.open(
        "data/sampleRoundData.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    dat = sut_.getData();
    foundZombie = sut_.findNearestZombie(dat.ashPos_, dat.zombies_);
    ASSERT_EQ(1, foundZombie.id_);
}

TEST_F(GameControllerShould, markHumansLostAndEndangeredDuringTheTriage)
{
    GameData dat;
    ifstream ifs;
    Human lostHuman(0,Position());
    Human endangeredHuman(1, Position());
    Human okHuman(2, Position());

    ifs.open(
        "data/lostHumanData.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    sut_.doTheTriage();
    dat = sut_.getData();
    ASSERT_EQ(Human::Category::Lost,
        dat.humans_.find(lostHuman)->cat_);
    ASSERT_EQ(Human::Category::Endangered,
        dat.humans_.find(endangeredHuman)->cat_);
    ASSERT_EQ(Human::Category::OK,
        dat.humans_.find(okHuman)->cat_);
    ASSERT_TRUE(sut_.atLeastOneHumanIsSave());
    dat.humans_.find(okHuman)->cat_ = Human::Category::Lost;
    ASSERT_FALSE(sut_.atLeastOneHumanIsSave(dat.humans_));
}

TEST_F(GameControllerShould, giveTheLowestRateToTheDenseZombies)
{
    GameData dat;
    ifstream ifs;
    vector<Zombie> dullZombies;
    vector<Zombie> interestingZombies;
    Zombie bestZombie;

    dullZombies.push_back(Zombie(
        9, Position(), Position()));
    dullZombies.push_back(Zombie(
        10, Position(), Position()));
    // interestingZombies.push_back(Zombie(
    //     16, Position(), Position()));
    interestingZombies.push_back(Zombie(
        0, Position(), Position()));
    interestingZombies.push_back(Zombie(
        1, Position(), Position()));

    ifs.open(
        "data/manyZombies.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    sut_.doTheTriage();
    sut_.rateZombies();
    dat = sut_.getData();
    for (auto zombie: dat.zombies_)
    {
        cout << "Zombie[" << zombie.id_ << "] "
            << zombie.appealFactor_ << endl;
    }
    for (auto dull: dullZombies)
    {
        for (auto interesting: interestingZombies)
        {
            ASSERT_GT(
                dat.zombies_.find(interesting)->appealFactor_,
                dat.zombies_.find(dull)->appealFactor_);
        }
    }
    bestZombie = sut_.findZombieWithHighestAppealFactor(
        dat.zombies_);
    ASSERT_EQ(1, bestZombie.id_);
}

TEST_F(GameControllerShould, calculateCenterOfMassFromPointsVector)
{
    vector<Position> positions;
    Position expectedCenter(3,2);
    Position center;

    positions.push_back(Position(2,1));
    positions.push_back(Position(3,0));
    positions.push_back(Position(4,2));
    positions.push_back(Position(3,5));

    center = sut_.centerOfMass(positions);
    ASSERT_EQ(expectedCenter.x_, center.x_);
    ASSERT_EQ(expectedCenter.y_, center.y_);
}

TEST_F(GameControllerShould, returnCorrectNumberOfNeighboursForZombies)
{
    GameData dat;
    ifstream ifs;
    vector<Zombie> zombieNeighbours1;
    vector<Zombie> zombieNeighbours2;
    Zombie zombie1(0, Position(), Position());
    Zombie zombie2(16, Position(), Position());

    ifs.open(
        "data/manyZombies.dat",
        std::ifstream::in);
    sut_.loadGameData(ifs);
    ifs.close();
    dat = sut_.getData();
    zombieNeighbours1 = sut_.selectZombiesFromNeighbourhood(
        *(dat.zombies_.find(zombie1)),
        dat.zombies_);
    zombieNeighbours2 = sut_.selectZombiesFromNeighbourhood(
        *(dat.zombies_.find(zombie2)),
        dat.zombies_);
    ASSERT_EQ(2, zombieNeighbours1.size());
    ASSERT_EQ(3, zombieNeighbours2.size());
}
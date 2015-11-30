# CodingGame-CodeVsZombies
Code from 28.11.2015 24h Coding Game contest "Code vs Zombies"

This code was ranked 530/3963 in the official reports. Proposed strategy is meant to be universal rather than based on specific test cases. The main algorithm is a mixture of PotentialFieldMethod, CenterOfMass formulas and a few heuristic factors.

This strategy favored human lives over zombie kill combos, which was good for most cases. However, in few tests a huge score could be gained by sacrificing all but one human in exchange for zombie multikill. This is well shown in the chart comparison of Fibonacci sequence with x^2 function:
http://www.wolframalpha.com/input/?i=plot[{Fibonacci[x]%2C+x^2}%2C{x%2C0%2C15}]


[REPLAYS]

https://www.codingame.com/replay/solo/67686418
https://www.codingame.com/replay/solo/67686424
https://www.codingame.com/replay/solo/67686434
https://www.codingame.com/replay/solo/67686458
https://www.codingame.com/replay/solo/67686465
https://www.codingame.com/replay/solo/67686470
https://www.codingame.com/replay/solo/67686472
https://www.codingame.com/replay/solo/67686481


[RULES]

This contest is a 24 hour optimization challenge. Please note that the available 24 hours does not reflect the difficulty of the challenge. Also it does not mean that you will have to code 24 hours in a row.

It is in fact quite simple to obtain a 100% score. However, we have made it possible for you to submit a solution several times across the next 24 hours, so that you may attempt to improve your code at any moment.

The players with the most points across all validator test cases at the end of the 24 hours will be the winners.


Destroy zombies quickly to earn points and make sure to keep the humans alive to get the highest score that you can manage.
  Rules
The game is played in a zone 16000 units wide by 9000 units high. You control a man named Ash, wielding a gun that lets him kill any zombie within a certain range around him.

Ash works as follows:

    Ash can be told to move to any point within the game zone by outputting a coordinate X Y. The top-left point is 0 0.
    Each turn, Ash will move exactly 1000 units towards the target coordinate, or onto the target coordinates if he is less than 1000 units away.
    If at the end of a turn, a zombie is within 2000 units of Ash, he will shoot that zombie and destroy it. More details on combat further down.


Other humans will be present in the game zone, but will not move. If zombies kill all of them, you lose the game and score 0 points for the current test case.

Zombies are placed around the game zone at the start of the game, they must be destroyed to earn points.

Zombies work as follows:

    Each turn, every zombie will target the closest human, including Ash, and step 400 units towards them. If the zombie is less than 400 units away, the human is killed and the zombie moves onto their coordinate.
    Two zombies may occupy the same coordinate.


The order in which actions happens in between two rounds is:

    Zombies move towards their targets.
    Ash moves towards his target.
    Any zombie within a 2000 unit range around Ash is destroyed.
    Zombies eat any human they share coordinates with.


Killing zombies earns you points. The number of points you get per zombie is subject to a few factors.

Scoring works as follows:

    A zombie is worth the number of humans still alive squared x10, not including Ash.
    If several zombies are destroyed during on the same round, the nth zombie killed's worth is multiplied by the (n+2)th number of the Fibonnacci sequence (1, 2, 3, 5, 8, and so on). As a consequence, you should kill the maximum amount of zombies during a same turn.

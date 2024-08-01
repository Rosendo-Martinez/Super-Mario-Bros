#include "../src/Animation.h"
#include <iostream>

int main()
{
    sf::Texture t;
    if (!t.create(300,100))
    {
        std::cout << "Could not create texture.\n";
        return 0;
    }

    // TERRIBLE FUCKING TESTS, BUT they got the job done

    const int frameCount = 3;
    const int speed = 10;
    int currentFrame = 1;
    Animation a("",t, frameCount, speed);

    // 0 updates, expect to be on animation frame 1
    if (!(a.getSprite().getTextureRect().left == 0)) {
        std:: cout << "T1: Error: animation should be at frame 1\n";
    }
    a.update();
    currentFrame++;
    // 1 updates, expect to be on animation frame 1
    if (!(a.getSprite().getTextureRect().left == 0)) {
        std:: cout << "T2: Error: animation should be at frame 1\n";
    }
    // after total 30 animation, animation should be at final animation
    while (currentFrame < 20) {
        a.update();
        currentFrame++;
    }
    // 20 updates, expect to be on animation frame 2
    if (!(a.getSprite().getTextureRect().left == 100)) {

        std:: cout << "T2.5: Error: animation should be at frame 2 " << a.getSprite().getTextureRect().left << " \n";
    }
    while (currentFrame < 30) {
        a.update();
        currentFrame++;
    }
    if (!(a.getSprite().getTextureRect().left == 200)) {
        std:: cout << "T3: Error: animation should be at frame 3 " << a.getSprite().getTextureRect().left << " \n";
    }
    // after total 31 animation, animation should be back at frame 1
    a.update();
    currentFrame++;
    if (!(a.getSprite().getTextureRect().left == 0)) {
        std:: cout << "T4: Error: animation should be at frame 1\n";
    }
}
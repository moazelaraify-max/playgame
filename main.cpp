#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>   // [جديد] نحتاجها لحساب الزوايا (sin, cos)
#include <cstdint> // [جديد] نحتاجها لاستخدام std::uint8_t

// [إضافة 1] هيكل بسيط يمثل الشظية الواحدة
struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity; // سرعة واتجاه الشظية
    float lifetime;        // كم ثانية ستعيش الشظية؟
};

bool rectIntersects(const sf::FloatRect& a, const sf::FloatRect& b)
{
    return a.position.x < b.position.x + b.size.x &&
           a.position.x + a.size.x > b.position.x &&
           a.position.y < b.position.y + b.size.y &&
           a.position.y + a.size.y > b.position.y;

}
int main()
{ 
    sf::RenderWindow window(sf::VideoMode({700, 500}), "My First SFML Game!");
    window.setFramerateLimit(60);
    
    sf::Texture background;
    if (!background.loadFromFile("background_mountains.png")){}
    sf::Sprite backgroundSprite(background);
    backgroundSprite.setScale({700.f / background.getSize().x, 500.f / background.getSize().y});
    
   
     sf::Texture player;
    if (!player.loadFromFile("player.png")){}
    sf::Sprite playerSprite(player);
    playerSprite.setPosition({250.f, 350.f});
    playerSprite.setScale({0.09f, 0.09f});
    
     sf::Texture enemy;
    if (!enemy.loadFromFile("enemy.png")){}

    
    sf::Music bgMusic;
    if(!bgMusic.openFromFile("play.wav"))
    {
         std::cout << "Error loading play.wav"<<std::endl;
    } 
    bgMusic.setLooping(true);
    bgMusic.setVolume(50);
    bgMusic.play();


    sf::SoundBuffer shootBuffer;
    if (!shootBuffer.loadFromFile("shoot.wav"))
    {
        std::cout << "Error loading shoot.wav" << std::endl;
    }
        sf::Sound shootSound(shootBuffer);

        sf::SoundBuffer explosionBuffer;
        if (!explosionBuffer.loadFromFile("explosion.wav"))
        {
            std::cout << "Error loading explosion.wav" <<std::endl;
        }
        sf::Sound explosion(explosionBuffer);

    float speed = 200.f;
    std::vector<sf::CircleShape> bullets;
    float shootTimer = 0.f;
    
std::vector<sf::Sprite> enemies;
std::vector<float> enemySpeeds;
const int numEnemy = 5;
std::srand(static_cast<unsigned int>(std::time(0)));
// [إضافة 2] قائمة لتخزين الانفجارات (الشظايا)
    std::vector<Particle> particles;

auto resetEnemy= [&](size_t index){
    float startx = 50+ std::rand() % 650;
    float starty = -1 *(std::rand () % 300);
    enemies[index].setPosition({startx, starty});
    float enemySpeed = (static_cast<float>(std::rand())) / static_cast<float>(RAND_MAX) * 100.f - 80.f;
    enemySpeeds[index] = enemySpeed;
};

for (int i = 0; i < numEnemy; ++i)
{
    sf::Sprite enemySprite(enemy);
    enemySprite.setScale({0.06, 0.06});
    enemies.push_back(enemySprite);
    enemySpeeds.push_back(0.f);
    resetEnemy(i);

}

    sf::Clock clock;
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        shootTimer += dt;

        for (auto event = window.pollEvent(); event; event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
        
        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) movement.x -= speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movement.x += speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) movement.y -= speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) movement.y += speed * dt;
        playerSprite.move(movement);
        
        sf::FloatRect playerBounds = playerSprite.getGlobalBounds();
        sf::Vector2f pos = playerSprite.getPosition();
        if (pos.x < 0) pos.x = 0;
        if (pos.y < 0) pos.y = 0;
        if (pos.x + playerBounds.size.x > 700) pos.x = 700 - playerBounds.size.x;
        if (pos.y + playerBounds.size.y > 500) pos.y = 500 - playerBounds.size.y;
        playerSprite.setPosition(pos);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && shootTimer >= 0.15f) 
        {
            sf::CircleShape bullet(5.f);
            bullet.setFillColor(sf::Color::Red);
            
            float bulletX = playerBounds.position.x + (playerBounds.size.x / 2) - 5.f; 
            float bulletY = playerBounds.position.y; 
            
            bullet.setPosition({bulletX, bulletY});
            bullets.push_back(bullet);
            shootSound.play();
            
            shootTimer = 0.f;
        }

        for (size_t i = 0; i < bullets.size(); i++)
        {
            bullets[i].move({0.f, -500.f * dt});  
            
            if (bullets[i].getPosition().y < -10)
            {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }

        for ( size_t i = 0; i< enemies.size(); i++)
        {
            enemies[i].move({0.f ,speed * dt});
            enemies[i].move({enemySpeeds[i] * dt, 0.f});
            sf::FloatRect enemyBounds = enemies[i].getGlobalBounds();
            sf::Vector2f enemyPos = enemies[i].getPosition();
            if (enemyPos.y > 600)
            {
                resetEnemy(i);             
            }
            
            if (enemyPos.x < -enemyBounds.size.x)
           {
            enemyPos.x = 700.f;
            enemies[i].setPosition(enemyPos);
           }

            else if (enemyPos.x > 700.f)
           {
            enemyPos.x = -enemyBounds.size.x;
            enemies[i].setPosition(enemyPos);
           }          
        }   
        
// --- [إضافة 3] تحديث الشظايا (الانفجارات) ---
        for (size_t i = 0; i < particles.size(); i++)
        {
            // تقليل العمر
            particles[i].lifetime -= dt;

            // إذا انتهى العمر، احذف الشظية
            if (particles[i].lifetime <= 0)
            {
                particles.erase(particles.begin() + i);
                i--;
                continue;
            }

            // تحريك الشظية
            particles[i].shape.move(particles[i].velocity * dt);

            // تغيير الشفافية (تتلاشى مع الوقت)
            sf::Color c = particles[i].shape.getFillColor();
            // 255 هو أقصى وضوح، نضربه في نسبة العمر المتبقي
            c.a = static_cast<std::uint8_t>(255 * particles[i].lifetime);
            particles[i].shape.setFillColor(c);
        }

        for (size_t i = 0; i < enemies.size();++i)
        {
            sf::FloatRect enemyBounds = enemies[i].getGlobalBounds();

            for (size_t j = 0; j < bullets.size(); ++j)
            {
                sf::FloatRect bulletBounds = bullets[j].getGlobalBounds();
                 
                if (rectIntersects(enemyBounds, bulletBounds ))
                {
                    // [إضافة 4] توليد الانفجار عند موقع التصادم
                    sf::Vector2f boomPos = enemies[i].getPosition();
                    
                    // ننشئ 20 شظية
                    for(int k=0; k<25; k++)
                    {
                        Particle p;
                        p.shape.setRadius(4.f); // حجم الشظية
                        p.shape.setFillColor(sf::Color(255, 345, 230)); // برتقالي
                        p.shape.setPosition(boomPos);
                        
                        // حساب زاوية وسرعة عشوائية لكل شظية
                        float angle = (std::rand() % 360) * 3.14159f / 180.f;
                        float speed = (std::rand() % 150) + 100.f; // سرعة بين 100 و 250
                        
                        p.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };
                        p.lifetime = 1.0f; // تعيش لمدة ثانية واحدة

                        particles.push_back(p);
                    }

                    explosion.play();
                    resetEnemy(i);
                    bullets.erase(bullets.begin()+j);
                    break;
                }
            }
        }
        sf::FloatRect currentplayerBounds = playerSprite.getGlobalBounds();
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            sf::FloatRect enemyBounds = enemies[i].getGlobalBounds();
            if (rectIntersects( currentplayerBounds, enemyBounds))
            {
                explosion.play();
                playerSprite.setPosition({350.f, 450.f});
                resetEnemy(i);
            }



        }

        window.clear(sf::Color(50, 50, 50));
        window.draw(backgroundSprite);
        window.draw(playerSprite);

        
        for (const auto& bullet : bullets)
        {
            window.draw(bullet);
        }
        

        for (const auto& enemy : enemies)
        {
            window.draw(enemy);
        }
        
        // [إضافة 5] رسم الشظايا
        for (const auto& p : particles)
        {
            window.draw(p.shape);
        }


        window.display();
    }

    return 0;
}
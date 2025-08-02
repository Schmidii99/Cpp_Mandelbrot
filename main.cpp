#include <complex>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <limits>

using std::string;

struct Complex {
    double im;
    double re;

    Complex(const double re, const double im) : im(im), re(re){}
    explicit Complex(const double x) : im(x), re(x){}
    [[nodiscard]] string to_string() const {
        return  std::to_string(re) + " + " + std::to_string(im) + "i";
    }

    Complex operator+(const Complex& c) const {
        return Complex{re + c.re, im + c.im};
    }

    void operator+=(const Complex c) {
        im += c.im;
        re += c.re;
    }

    [[nodiscard]] Complex square() const {
        if (this->re == 0 || this->im == 0)
            return Complex{pow(this->re + this->im, 2), 0};

        return Complex{pow(this->re, 2) - pow(this->im,2), 2 * this->im * this->re };
    }

    [[nodiscard]] double magnitude() const {
        return sqrt(pow(this->im, 2) + pow(this->re, 2));
    }
};

int steps_to_explode(Complex c, int max_steps = 50, int exploded_on = 1000) {
    int steps = 0;
    Complex z{0};

    while (steps < max_steps && z.magnitude() < exploded_on) {
        steps++;
        // std::cout << "squared: " << z.square().to_string() << " Iteration: " << (z.square() + c).to_string() << std::endl;
        z = z.square() + c;
    }

    return steps;
}


constexpr u_int16_t WIDTH = 1400;
constexpr u_int16_t HEIGHT = 800;

int main() {
    // create the window
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot");
    bool rendered = false;

    double_t MAX_X = 2;
    double_t MIN_X = -3;
    double_t MAX_Y = 2;
    double_t MIN_Y = -2;
    int32_t MAX_ITERATIONS = 50;
    int8_t ITERATION_INCREASE = 5;

    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event{};
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed) {
                std::cout << event.mouseButton.x << ":" << event.mouseButton.y << std::endl;
                const double x = static_cast<double>(event.mouseButton.x + 1) / static_cast<double>(WIDTH + 1) * (MAX_X - MIN_X) + MIN_X;
                const double y = static_cast<double>(event.mouseButton.y + 1) / static_cast<double>(HEIGHT + 1) * (MAX_Y - MIN_Y) + MIN_Y;

                const double_t size_x = MAX_X - MIN_X;
                const double_t size_y = MAX_Y - MIN_Y;

                if (event.mouseButton.button == sf::Mouse::Left){
                    // zoom in
                    MAX_X = x + size_x / 3;
                    MIN_X = x - size_x / 3;
                    MAX_Y = y + size_y / 3;
                    MIN_Y = y - size_y / 3;
                    MAX_ITERATIONS += ITERATION_INCREASE;
                } else {
                    // zoom out
                    MAX_X = x + size_x * 1.3;
                    MIN_X = x - size_x * 1.3;
                    MAX_Y = y + size_y * 1.3;
                    MIN_Y = y - size_y * 1.3;
                    MAX_ITERATIONS -= ITERATION_INCREASE;
                }

                rendered = false;
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::Up) {
                ITERATION_INCREASE += 5;
                rendered = false;
            }
        }

        if (rendered)
            continue;

        // clear the window with black color
        window.clear(sf::Color::Black);

        auto* pixels = new sf::Uint8[WIDTH*HEIGHT*4];

        sf::Texture texture;
        texture.create(WIDTH, HEIGHT);

        sf::Sprite sprite(texture); // needed to draw the texture on screen

        for(int i = 0; i < WIDTH*HEIGHT*4; i += 4) {
            const int16_t pixel_x = (i / 4) % WIDTH;
            const int16_t pixel_y = (i / 4) / WIDTH;
            // map x from 0 - WIDTH -> -2 - 1
            const double x = static_cast<double>(pixel_x + 1) / static_cast<double>(WIDTH + 1) * (MAX_X - MIN_X) + MIN_X;
            const double y = static_cast<double>(pixel_y + 1) / static_cast<double>(HEIGHT + 1) * (MAX_Y - MIN_Y) + MIN_Y;

            Complex c{x, y};
            const int steps = steps_to_explode(c, MAX_ITERATIONS);

            pixels[i] = 255;
            pixels[i+1] = 255;
            pixels[i+2] = 255;
            pixels[i+3] = 255 * (1 - steps / MAX_ITERATIONS);
        }

        texture.update(pixels);
        window.draw(sprite);

        delete pixels;

        // end the current frame
        window.display();
        rendered = true;
    }

    return 0;
}

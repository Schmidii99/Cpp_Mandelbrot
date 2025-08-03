#include <complex>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <thread>

using std::string;

struct Complex {
    double re;
    double im;

    Complex(const double re, const double im) : re(re), im(im){}
    explicit Complex(const double x) : re(x), im(x){}

    Complex operator+(const Complex& c) const {
        return Complex{re + c.re, im + c.im};
    }

    void operator+=(const Complex c) {
        im += c.im;
        re += c.re;
    }

    [[nodiscard]] Complex square() const {
        if (re == 0 || im == 0)
            return Complex{(re + im) * (re + im), 0.0};

        return Complex{re * re - im * im, 2 * im * re };
    }

    [[nodiscard]] double magnitude() const {
        return sqrt(pow(im, 2) + pow(re, 2));
    }
};

u_int16_t steps_to_explode(const Complex c, const int16_t max_steps = 50, const int16_t exploded_on = 1000) {
    u_int16_t steps = 0;
    Complex z{0.0};

    while (steps < max_steps && z.magnitude() < exploded_on) {
        steps++;
        z = z.square() + c;
    }

    return steps;
}

void set_rgba(sf::Uint8* pixels, int32_t index, u_int16_t steps, int32_t MAX_ITERATIONS) {
    const std::size_t h1 = std::hash<string>{}(std::to_string(steps) + std::to_string(steps) + std::to_string(steps));
    // std::cout << "Hash: " << h1 << std::endl;
    pixels[index] = h1 % 239;
    pixels[index+1] = h1 % 241;
    pixels[index+2] = h1 % 251;
    pixels[index+3] = steps == MAX_ITERATIONS ? 0 : 255;
}

void calculate_pixels(sf::Uint8* pixels, int32_t index, const int32_t MAX, const u_int16_t WIDTH, const u_int16_t HEIGHT, const int32_t MAX_ITERATIONS, const double_t MAX_X, const double_t MIN_X, const double_t MAX_Y, const double_t MIN_Y) {
    for(; index < MAX; index += 4) {
        const u_int16_t pixel_x = (index / 4) % WIDTH;
        const u_int16_t pixel_y = (index / 4) / WIDTH;
        // map pixel cords to actual x/y coordinates
        const double x = static_cast<double>(pixel_x + 1) / static_cast<double>(WIDTH + 1) * (MAX_X - MIN_X) + MIN_X;
        const double y = static_cast<double>(pixel_y + 1) / static_cast<double>(HEIGHT + 1) * (MAX_Y - MIN_Y) + MIN_Y;

        Complex c{x, y};
        const u_int16_t steps = steps_to_explode(c, MAX_ITERATIONS);

        set_rgba(pixels, index, steps, MAX_ITERATIONS);
    }
}


constexpr u_int16_t WIDTH = 1400;
constexpr u_int16_t HEIGHT = 800;
constexpr u_int8_t THREAD_COUNT = 4;

using namespace std;
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

    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
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

        // check if scope changed
        if (rendered)
            continue;

        // clear the window with black color
        window.clear(sf::Color::Black);

        auto* pixels = new sf::Uint8[WIDTH*HEIGHT*4];

        sf::Texture texture;
        texture.create(WIDTH, HEIGHT);

        sf::Sprite sprite(texture); // needed to draw the texture on screen

        auto* threads = new thread[THREAD_COUNT];
        int chunk_size = WIDTH*HEIGHT*4 / THREAD_COUNT;

        for (int i = 0; i < THREAD_COUNT; i++) {
            threads[i] = thread(calculate_pixels,
                                pixels,
                                i * chunk_size,
                                i * chunk_size + chunk_size,
                                WIDTH, HEIGHT,
                                MAX_ITERATIONS,
                                MAX_X, MIN_X, MAX_Y, MIN_Y);
        }
        // wait for all threads to finish
        for (int i = 0; i < THREAD_COUNT; i++) {
            threads[i].join();
        }

        delete[] threads;

        texture.update(pixels);
        window.draw(sprite);

        delete pixels;

        // end the current frame
        window.display();
        rendered = true;
    }

    return 0;
}

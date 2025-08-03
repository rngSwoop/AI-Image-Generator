#define _CRT_SECURE_NO_WARNINGS
#include "ImageGenerator.h"
#include <chrono>
#include <fstream>

using json = nlohmann::json;

ImageGenerator::ImageGenerator() : window(sf::VideoMode({ 1024, 768 }), "AI Image Generator"),
currentState(AppState::INPUT_SCREEN),
selectedStyle(StyleMode::NONE),
selectedModel(APIModel::REALISM),
promptActive(false),
backgroundColor(sf::Color(40, 40, 40)),
buttonColor(sf::Color(70, 70, 70)),
buttonHoverColor(sf::Color(90, 90, 90)),
selectedButtonColor(sf::Color(100, 150, 200)),
disabledButtonColor(sf::Color(50, 50, 50)),
promptLabel(font),
promptText(font),
ghibliLabel(font),
photorealisticLabel(font),
generateLabel(font),
newImageLabel(font),
loadingText(font),
cursorPosition(0),
cursorVisible(true) {

    if (!font.openFromFile("Yrsa-Regular.ttf")) {
        // Try to load a system font as fallback
        if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            std::cout << "Warning: Could not load Yrsa-Regular.ttf or arial.ttf, using default font" << std::endl;
        }
    }

    // Initialize cursor
    cursor.setSize({ 2, 20 });
    cursor.setFillColor(sf::Color::White);

    modelNames = { "Realism", "Aesthetic" };
    initializeUI();
}

void ImageGenerator::run() {
    while (window.isOpen()) {
        handleEvents();
        render();
    }
}

void ImageGenerator::runCommandLine(const std::string& prompt, const std::string& style) {
    std::cout << "Processing prompt: " << prompt << std::endl;
    std::cout << "Style: " << style << std::endl;

    // Set the prompt and style
    userPrompt = prompt;

    // Convert style string to enum
    if (style == "studio_ghibli") selectedStyle = StyleMode::STUDIO_GHIBLI;
    else if (style == "photorealistic") selectedStyle = StyleMode::PHOTOREALISTIC;
    else selectedStyle = StyleMode::NONE;

    // Generate and display image
    generateImage();

    // Wait for generation to complete
    while (currentState == AppState::LOADING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    currentState = AppState::IMAGE_DISPLAY;
    run(); // Show GUI with generated image
}
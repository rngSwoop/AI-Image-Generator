#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <future>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

enum class AppState {
    INPUT_SCREEN,
    LOADING,
    IMAGE_DISPLAY
};

enum class StyleMode {
    NONE,
    STUDIO_GHIBLI,
    PHOTOREALISTIC
};

enum class APIModel {
    REALISM,    // FLUX schnell for photorealistic, fast generation
    AESTHETIC   // Playground v2.5 for artistic, aesthetic quality
};

class ImageGenerator {
private:
    sf::RenderWindow window;
    sf::Font font;
    AppState currentState;

    // Input screen elements
    sf::Text promptLabel;
    sf::RectangleShape promptBox;
    sf::Text promptText;
    std::string userPrompt;
    bool promptActive;

    // Style selection
    sf::RectangleShape ghibliButton;
    sf::Text ghibliLabel;
    sf::RectangleShape photorealisticButton;
    sf::Text photorealisticLabel;
    StyleMode selectedStyle;

    // API Model selection
    std::vector<sf::RectangleShape> modelButtons;
    std::vector<sf::Text> modelLabels;
    APIModel selectedModel;
    std::vector<std::string> modelNames;

    // Generate button
    sf::RectangleShape generateButton;
    sf::Text generateLabel;

    // Image display
    sf::Texture imageTexture;
    sf::Sprite imageSprite{ imageTexture };
    sf::RectangleShape newImageButton;
    sf::Text newImageLabel;

    // Loading
    sf::Text loadingText;

    // Colors
    sf::Color backgroundColor;
    sf::Color buttonColor;
    sf::Color buttonHoverColor;
    sf::Color selectedButtonColor;
    sf::Color disabledButtonColor;

    // Private helper methods
    void initializeUI();
    void handleInputScreenEvents(sf::Event& event);
    void handleImageDisplayEvents(sf::Event& event);
    void updateStyleButtons();
    void updateModelButtons();
    void updateButtonHovers(sf::Vector2i mousePos);
    void generateImage();
    void renderInputScreen();
    void renderLoadingScreen();
    void renderImageDisplay();

    // API methods
    std::string makeAPIRequest(const std::string& prompt, const std::string& styleModifier, APIModel model);
    std::string pollRequestStatus(const std::string& requestId, APIModel model);
    bool downloadImage(const std::string& imageUrl, const std::string& filename);
    std::string getStylePromptModifier(StyleMode style);

    // HTTP callback for curl
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

public:
    ImageGenerator();
    void handleEvents();
    void render();
    void run();
    void runCommandLine(const std::string& prompt, const std::string& style);
};
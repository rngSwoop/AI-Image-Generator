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
    // Legacy styles (for backward compatibility)
    STUDIO_GHIBLI,
    PHOTOREALISTIC,
    // Artistic styles
    IMPRESSIONISM,
    ABSTRACT_EXPRESSIONISM,
    CUBISM,
    ART_DECO,
    POP_ART,
    REALISM_ART,
    EXPRESSIONISM,
    BAROQUE,
    FAUVISM,
    NEOCLASSICISM,
    FUTURISM,
    SURREALISM,
    RENAISSANCE,
    ACADEMIC_ART,
    ANALYTICAL_ART,
    BAUHAUS,
    CONCEPTUAL_ART,
    CONSTRUCTIVISM,
    DADA,
    GEOMETRIC_ABSTRACTION,
    MINIMALISM_ART,
    NEO_IMPRESSIONISM,
    POST_IMPRESSIONISM,
    // Interior design styles
    MID_CENTURY_MODERN,
    BOHEMIAN,
    MINIMALISM_DESIGN,
    SCANDINAVIAN,
    ART_DECO_DESIGN,
    FARMHOUSE,
    INDUSTRIAL,
    CONTEMPORARY,
    TRADITIONAL,
    RUSTIC,
    TRANSITIONAL,
    FRENCH_COUNTRY,
    JAPANDI,
    MEDITERRANEAN,
    SHABBY_CHIC,
    ECLECTIC,
    REGENCY,
    COASTAL,
    MAXIMALISM,
    // Gaming & Tech styles
    CYBERPUNK,
    SYNTHWAVE,
    PIXEL_ART,
    ANIME_MANGA,
    SCI_FI_TECH,
    RETRO_GAMING,
    // Entertainment styles
    MOVIE_POSTER,
    FILM_NOIR,
    CONCERT_POSTER,
    SPORTS_MEMORABILIA,
    VINTAGE_CINEMA,
    // Professional styles
    CORPORATE_MODERN,
    ABSTRACT_CORPORATE,
    NATURE_ZEN,
    // Specialty Room styles
    CULINARY_KITCHEN,
    LIBRARY_ACADEMIC,
    FITNESS_GYM,
    KIDS_CARTOON,
    // Landscape styles
    PHOTOREALISTIC_LANDSCAPES,
    SEASONAL_LANDSCAPES,
    WEATHER_MOODS,
    TIME_OF_DAY
};

enum class APIModel {
    REALISM,        // FLUX schnell for photorealistic, fast generation
    AESTHETIC,      // Playground v2.5 for artistic, aesthetic quality
    ARTISTIC,       // FLUX LoRA for artistic styles with high quality
    GAMING_TECH,    // FLUX LoRA for gaming and tech aesthetics
    ENTERTAINMENT,  // FLUX LoRA for movie posters, film noir, etc.
    PROFESSIONAL,   // FLUX schnell for corporate and professional themes
    SPECIALTY_ROOMS,// FLUX LoRA for themed room content
    LANDSCAPES      // FLUX schnell for photorealistic nature scenes
};

class ImageGenerator {
private:
    sf::Font font;
    sf::RenderWindow window;
    AppState currentState;

    // View management for proper aspect ratio
    sf::View logicalView;
    static const unsigned int LOGICAL_WIDTH = 1024;
    static const unsigned int LOGICAL_HEIGHT = 768;

    // Input screen elements
    sf::Text promptLabel;
    sf::RectangleShape promptBox;
    sf::Text promptText;
    std::string userPrompt;
    bool promptActive;

    // Cursor elements
    size_t cursorPosition;
    sf::RectangleShape cursor;
    sf::Clock cursorClock;
    bool cursorVisible;

    // Legacy style selection (for backward compatibility)
    sf::RectangleShape ghibliButton;
    sf::Text ghibliLabel;
    sf::RectangleShape photorealisticButton;
    sf::Text photorealisticLabel;
    StyleMode selectedStyle;

    // New hierarchical style system
    std::vector<sf::RectangleShape> categoryStyleButtons;
    std::vector<sf::Text> categoryStyleLabels;
    std::vector<StyleMode> currentCategoryStyles;
    std::vector<std::string> currentCategoryStyleNames;

    // Artistic style buttons (existing)
    std::vector<sf::RectangleShape> artisticStyleButtons;
    std::vector<sf::Text> artisticStyleLabels;
    std::vector<sf::RectangleShape> interiorStyleButtons;
    std::vector<sf::Text> interiorStyleLabels;
    std::vector<StyleMode> artisticStyles;
    std::vector<StyleMode> interiorStyles;
    std::vector<std::string> artisticStyleNames;
    std::vector<std::string> interiorStyleNames;

    // Gaming & Tech styles
    std::vector<StyleMode> gamingTechStyles;
    std::vector<std::string> gamingTechStyleNames;

    // Entertainment styles
    std::vector<StyleMode> entertainmentStyles;
    std::vector<std::string> entertainmentStyleNames;

    // Professional styles
    std::vector<StyleMode> professionalStyles;
    std::vector<std::string> professionalStyleNames;

    // Specialty Room styles
    std::vector<StyleMode> specialtyRoomStyles;
    std::vector<std::string> specialtyRoomStyleNames;

    // Landscape styles
    std::vector<StyleMode> landscapeStyles;
    std::vector<std::string> landscapeStyleNames;

    // Group labels
    sf::Text artisticGroupLabel;
    sf::Text interiorGroupLabel;
    sf::Text stylesGroupLabel;
    sf::Text categoryGroupLabel;

    // API Model selection (now 8 categories)
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

    // Scrolling for styles
    float artisticScrollOffset;
    bool artisticScrollActive;
    sf::RectangleShape artisticScrollArea;

    // Colors
    sf::Color backgroundColor;
    sf::Color buttonColor;
    sf::Color buttonHoverColor;
    sf::Color selectedButtonColor;
    sf::Color disabledButtonColor;

    // Private helper methods
    void initializeUI();
    void initializeArtisticStyles();
    void initializeAllCategoryStyles();
    void setupView();
    void handleWindowResize();
    sf::Vector2f getLogicalMousePosition(sf::Vector2i screenPos);
    void handleInputScreenEvents(sf::Event& event);
    void handleImageDisplayEvents(sf::Event& event);
    void updateStyleButtons();
    void updateModelButtons();
    void updateButtonHovers(sf::Vector2f mousePos);
    void updateCursorPosition();
    void handleScroll(sf::Event& event);
    void updateArtisticButtonPositions();
    void updateCategoryStylePositions();
    void generateImage();
    void renderInputScreen();
    void renderLoadingScreen();
    void renderImageDisplay();

    // API methods
    std::string makeAPIRequest(const std::string& prompt, const std::string& styleModifier, APIModel model);
    std::string pollRequestStatus(const std::string& requestId, APIModel model);
    bool downloadImage(const std::string& imageUrl, const std::string& filename);
    std::string getStylePromptModifier(StyleMode style);
    APIModel getAPIForModel(APIModel selectedModel);

    // HTTP callback for curl
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

public:
    ImageGenerator();
    void handleEvents();
    void render();
    void run();
    void runCommandLine(const std::string& prompt, const std::string& style);
};
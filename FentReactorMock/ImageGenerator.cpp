#include "ImageGenerator.h"
#include <chrono>
#include <fstream>

using json = nlohmann::json;

ImageGenerator::ImageGenerator() : window(sf::VideoMode({ 1024, 768 }), "AI Image Generator", sf::Style::Default),
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
artisticGroupLabel(font),
interiorGroupLabel(font),
stylesGroupLabel(font),
categoryGroupLabel(font),
globalOrientation(OrientationMode::PORTRAIT),
orientationLabel(font),
galleryLabel(font),
saveImageLabel(font),
backToMainLabel(font),
portraitTabLabel(font),
landscapeTabLabel(font),
galleryHeaderLabel(font),
galleryInfoLabel(font),
showingPortraitGallery(true),
galleryScrollOffset(0),
artisticScrollOffset(0),
artisticScrollActive(false),
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

    // Setup view for proper aspect ratio handling
    setupView();

    // Initialize all category models - 8 total categories
    modelNames = { "Realism", "Aesthetic", "Artistic", "Gaming & Tech", "Entertainment", "Professional", "Specialty Rooms", "Landscapes" };

    initializeArtisticStyles();
    initializeAllCategoryStyles();
    loadSavedImages();
    initializeUI();
}

void ImageGenerator::setupView() {
    // Create a basic logical view
    logicalView = sf::View(sf::Vector2f(LOGICAL_WIDTH / 2.0f, LOGICAL_HEIGHT / 2.0f),
        sf::Vector2f(static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)));

    // Apply the view
    window.setView(logicalView);

    // Enable key repeat for text input
    window.setKeyRepeatEnabled(true);
}

void ImageGenerator::handleWindowResize() {
    // For now, just maintain the basic view without complex viewport manipulation
    // This avoids the FloatRect constructor issues we encountered before
    window.setView(logicalView);
}

sf::Vector2f ImageGenerator::getLogicalMousePosition(sf::Vector2i screenPos) {
    // Convert screen coordinates to logical coordinates
    return window.mapPixelToCoords(screenPos);
}

void ImageGenerator::initializeAllCategoryStyles() {
    // Gaming & Tech styles
    gamingTechStyles = {
        StyleMode::CYBERPUNK, StyleMode::SYNTHWAVE, StyleMode::PIXEL_ART,
        StyleMode::ANIME_MANGA, StyleMode::SCI_FI_TECH, StyleMode::RETRO_GAMING
    };

    gamingTechStyleNames = {
        "Cyberpunk", "Synthwave", "Pixel Art",
        "Anime/Manga", "Sci-Fi Tech", "Retro Gaming"
    };

    // Entertainment styles
    entertainmentStyles = {
        StyleMode::MOVIE_POSTER, StyleMode::FILM_NOIR, StyleMode::CONCERT_POSTER,
        StyleMode::SPORTS_MEMORABILIA, StyleMode::VINTAGE_CINEMA
    };

    entertainmentStyleNames = {
        "Movie Poster", "Film Noir", "Concert Poster",
        "Sports Memorabilia", "Vintage Cinema"
    };

    // Professional styles
    professionalStyles = {
        StyleMode::CORPORATE_MODERN, StyleMode::ABSTRACT_CORPORATE, StyleMode::NATURE_ZEN
    };

    professionalStyleNames = {
        "Corporate Modern", "Abstract Corporate", "Nature/Zen"
    };

    // Specialty Room styles
    specialtyRoomStyles = {
        StyleMode::CULINARY_KITCHEN, StyleMode::LIBRARY_ACADEMIC,
        StyleMode::FITNESS_GYM, StyleMode::KIDS_CARTOON
    };

    specialtyRoomStyleNames = {
        "Culinary/Kitchen", "Library/Academic",
        "Fitness/Gym", "Kids/Cartoon"
    };

    // Landscape styles
    landscapeStyles = {
        StyleMode::PHOTOREALISTIC_LANDSCAPES, StyleMode::SEASONAL_LANDSCAPES,
        StyleMode::WEATHER_MOODS, StyleMode::TIME_OF_DAY
    };

    landscapeStyleNames = {
        "Photorealistic Landscapes", "Seasonal Landscapes",
        "Weather Moods", "Time of Day"
    };
}

void ImageGenerator::initializeArtisticStyles() {
    // Artistic styles (existing)
    artisticStyles = {
        StyleMode::IMPRESSIONISM, StyleMode::ABSTRACT_EXPRESSIONISM, StyleMode::CUBISM,
        StyleMode::ART_DECO, StyleMode::POP_ART, StyleMode::REALISM_ART,
        StyleMode::EXPRESSIONISM, StyleMode::BAROQUE, StyleMode::FAUVISM,
        StyleMode::NEOCLASSICISM, StyleMode::FUTURISM, StyleMode::SURREALISM,
        StyleMode::RENAISSANCE, StyleMode::ACADEMIC_ART, StyleMode::ANALYTICAL_ART,
        StyleMode::BAUHAUS, StyleMode::CONCEPTUAL_ART, StyleMode::CONSTRUCTIVISM,
        StyleMode::DADA, StyleMode::GEOMETRIC_ABSTRACTION, StyleMode::MINIMALISM_ART,
        StyleMode::NEO_IMPRESSIONISM, StyleMode::POST_IMPRESSIONISM
    };

    artisticStyleNames = {
        "Impressionism", "Abstract Expressionism", "Cubism",
        "Art Deco", "Pop Art", "Photorealistic Art",
        "Expressionism", "Baroque", "Fauvism",
        "Neoclassicism", "Futurism", "Surrealism",
        "Renaissance", "Academic Art", "Analytical Art",
        "Bauhaus", "Conceptual Art", "Constructivism",
        "Dada", "Geometric Abstraction", "Minimalism",
        "Neo-Impressionism", "Post-Impressionism"
    };

    // Interior design styles (existing)
    interiorStyles = {
        StyleMode::MID_CENTURY_MODERN, StyleMode::BOHEMIAN, StyleMode::MINIMALISM_DESIGN,
        StyleMode::SCANDINAVIAN, StyleMode::ART_DECO_DESIGN, StyleMode::FARMHOUSE,
        StyleMode::INDUSTRIAL, StyleMode::CONTEMPORARY, StyleMode::TRADITIONAL,
        StyleMode::RUSTIC, StyleMode::TRANSITIONAL, StyleMode::FRENCH_COUNTRY,
        StyleMode::JAPANDI, StyleMode::MEDITERRANEAN, StyleMode::SHABBY_CHIC,
        StyleMode::ECLECTIC, StyleMode::REGENCY, StyleMode::COASTAL, StyleMode::MAXIMALISM
    };

    interiorStyleNames = {
        "Mid-Century Modern", "Bohemian", "Minimalism",
        "Scandinavian", "Art Deco", "Farmhouse",
        "Industrial", "Contemporary", "Traditional",
        "Rustic", "Transitional", "French Country",
        "Japandi", "Mediterranean", "Shabby Chic",
        "Eclectic", "Regency", "Coastal", "Maximalism"
    };
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

std::string ImageGenerator::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

std::string ImageGenerator::getCategoryName(APIModel model) {
    switch (model) {
    case APIModel::REALISM: return "Realism";
    case APIModel::AESTHETIC: return "Aesthetic";
    case APIModel::ARTISTIC: return "Artistic";
    case APIModel::GAMING_TECH: return "Gaming & Tech";
    case APIModel::ENTERTAINMENT: return "Entertainment";
    case APIModel::PROFESSIONAL: return "Professional";
    case APIModel::SPECIALTY_ROOMS: return "Specialty Rooms";
    case APIModel::LANDSCAPES: return "Landscapes";
    default: return "Unknown";
    }
}

std::string ImageGenerator::getStyleName(StyleMode style) {
    switch (style) {
    case StyleMode::STUDIO_GHIBLI: return "Studio Ghibli";
    case StyleMode::PHOTOREALISTIC: return "Photorealistic";
    case StyleMode::IMPRESSIONISM: return "Impressionism";
    case StyleMode::ABSTRACT_EXPRESSIONISM: return "Abstract Expressionism";
    case StyleMode::CUBISM: return "Cubism";
    case StyleMode::CYBERPUNK: return "Cyberpunk";
    case StyleMode::SYNTHWAVE: return "Synthwave";
    case StyleMode::PIXEL_ART: return "Pixel Art";
    case StyleMode::ANIME_MANGA: return "Anime/Manga";
    case StyleMode::SCI_FI_TECH: return "Sci-Fi Tech";
    case StyleMode::RETRO_GAMING: return "Retro Gaming";
    case StyleMode::MOVIE_POSTER: return "Movie Poster";
    case StyleMode::FILM_NOIR: return "Film Noir";
    case StyleMode::CONCERT_POSTER: return "Concert Poster";
    case StyleMode::SPORTS_MEMORABILIA: return "Sports Memorabilia";
    case StyleMode::VINTAGE_CINEMA: return "Vintage Cinema";
    case StyleMode::CORPORATE_MODERN: return "Corporate Modern";
    case StyleMode::ABSTRACT_CORPORATE: return "Abstract Corporate";
    case StyleMode::NATURE_ZEN: return "Nature/Zen";
    case StyleMode::CULINARY_KITCHEN: return "Culinary/Kitchen";
    case StyleMode::LIBRARY_ACADEMIC: return "Library/Academic";
    case StyleMode::FITNESS_GYM: return "Fitness/Gym";
    case StyleMode::KIDS_CARTOON: return "Kids/Cartoon";
    case StyleMode::PHOTOREALISTIC_LANDSCAPES: return "Photorealistic Landscapes";
    case StyleMode::SEASONAL_LANDSCAPES: return "Seasonal Landscapes";
    case StyleMode::WEATHER_MOODS: return "Weather Moods";
    case StyleMode::TIME_OF_DAY: return "Time of Day";
        // Add more style mappings as needed
    case StyleMode::NONE: return "None";
    default: return "Custom Style";
    }
}

void ImageGenerator::saveCurrentImage() {
    if (currentGeneratedImagePath.empty()) {
        std::cout << "No image to save" << std::endl;
        return;
    }

    // Check if we need to remove old images
    if (savedImages.size() >= MAX_SAVED_IMAGES) {
        cleanupOldestImages();
    }

    // Create saved directory structure
    std::filesystem::create_directories("saved/portrait");
    std::filesystem::create_directories("saved/landscape");

    // Generate filename
    std::string timestamp = getCurrentTimestamp();
    std::string orientation = (globalOrientation == OrientationMode::PORTRAIT) ? "portrait" : "landscape";
    std::string savedFilename = "saved/" + orientation + "/" + orientation + "_" + timestamp + ".jpg";

    // Copy the generated image to saved location
    try {
        std::filesystem::copy_file(currentGeneratedImagePath, savedFilename);
        std::cout << "Image saved to: " << savedFilename << std::endl;

        // Create saved image metadata
        SavedImage savedImg(
            savedFilename,
            userPrompt,
            getCategoryName(selectedModel),
            getStyleName(selectedStyle),
            timestamp,
            (globalOrientation == OrientationMode::LANDSCAPE)
        );

        savedImages.push_back(savedImg);
        saveSavedImagesMetadata();

        std::cout << "Saved image metadata. Total saved: " << savedImages.size() << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "Error saving image: " << e.what() << std::endl;
    }
}

void ImageGenerator::cleanupOldestImages() {
    if (savedImages.empty()) return;

    // Find oldest image
    auto oldest = std::min_element(savedImages.begin(), savedImages.end(),
        [](const SavedImage& a, const SavedImage& b) {
            return a.timestamp < b.timestamp;
        });

    if (oldest != savedImages.end()) {
        // Delete the file
        try {
            std::filesystem::remove(oldest->filename);
            std::cout << "Removed old image: " << oldest->filename << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Error removing old image: " << e.what() << std::endl;
        }

        // Remove from vector
        savedImages.erase(oldest);
    }
}

void ImageGenerator::loadSavedImages() {
    savedImages.clear();

    std::ifstream file("saved/saved_images.json");
    if (!file.is_open()) {
        std::cout << "No saved images metadata found, starting fresh" << std::endl;
        return;
    }

    try {
        json j;
        file >> j;

        for (const auto& item : j["saved_images"]) {
            SavedImage img;
            img.filename = item["filename"];
            img.prompt = item["prompt"];
            img.category = item["category"];
            img.style = item["style"];
            img.timestamp = item["timestamp"];
            img.isLandscape = item["isLandscape"];

            // Verify file still exists
            if (std::filesystem::exists(img.filename)) {
                savedImages.push_back(img);
            }
        }

        std::cout << "Loaded " << savedImages.size() << " saved images" << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "Error loading saved images: " << e.what() << std::endl;
    }
}

void ImageGenerator::saveSavedImagesMetadata() {
    std::filesystem::create_directories("saved");

    json j;
    j["saved_images"] = json::array();

    for (const auto& img : savedImages) {
        json imgJson;
        imgJson["filename"] = img.filename;
        imgJson["prompt"] = img.prompt;
        imgJson["category"] = img.category;
        imgJson["style"] = img.style;
        imgJson["timestamp"] = img.timestamp;
        imgJson["isLandscape"] = img.isLandscape;
        j["saved_images"].push_back(imgJson);
    }

    std::ofstream file("saved/saved_images.json");
    file << j.dump(4);
    std::cout << "Saved images metadata updated" << std::endl;
}

std::vector<SavedImage> ImageGenerator::getCurrentGalleryImages() {
    std::vector<SavedImage> filtered;

    for (const auto& img : savedImages) {
        if (showingPortraitGallery && !img.isLandscape) {
            filtered.push_back(img);
        }
        else if (!showingPortraitGallery && img.isLandscape) {
            filtered.push_back(img);
        }
    }

    // Sort by timestamp (newest first)
    std::sort(filtered.begin(), filtered.end(),
        [](const SavedImage& a, const SavedImage& b) {
            return a.timestamp > b.timestamp;
        });

    return filtered;
}

void ImageGenerator::updateGalleryDisplay() {
    auto currentImages = getCurrentGalleryImages();

    std::string countText = std::to_string(currentImages.size()) + " saved " +
        (showingPortraitGallery ? "portrait" : "landscape") + " images";

    if (currentImages.empty()) {
        countText = "No saved " + std::string(showingPortraitGallery ? "portrait" : "landscape") + " images";
    }

    galleryInfoLabel.setString(countText);
    sf::FloatRect infoBounds = galleryInfoLabel.getLocalBounds();
    galleryInfoLabel.setPosition({ (1024 - infoBounds.size.x) / 2, 300 });
}
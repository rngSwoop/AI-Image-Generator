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
viewingFromGallery(false),
artisticScrollOffset(0),
artisticScrollActive(false),
cursorPosition(0),
cursorVisible(true),
loadingSpinner(30.0f), // 30 pixel radius
spinnerRotation(0.0f),
backToImageLabel(font),
hasGeneratedImage(false),
deleteImageLabel(font),
galleryFullWarning(font),
showGalleryFullWarning(false),
backToGalleryLabel(font),
imageSavedIndicator(font),
showImageSavedIndicator(false),
imageAlreadySavedCache(false),
imageAlreadySavedCacheValid(false) {

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

    // Initialize loading spinner
    loadingSpinner.setRadius(20.0f); // Reduced from 30.0f
    loadingSpinner.setPointCount(8); // Octagon for smoother spinning
    loadingSpinner.setFillColor(sf::Color::Transparent);
    loadingSpinner.setOutlineThickness(3); // Reduced from 4
    loadingSpinner.setOutlineColor(sf::Color(100, 150, 200));
    loadingSpinner.setOrigin({ 20.0f, 20.0f }); // Updated for new radius
    loadingSpinner.setPosition({ 512, 320 }); // Moved up to be above text
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
    case StyleMode::NONE: return "None";
    default: return "Custom Style";
    }
}

void ImageGenerator::saveCurrentImage() {
    if (currentGeneratedImagePath.empty()) {
        std::cout << "No image to save" << std::endl;
        return;
    }

    // Check if already saved
    if (isImageAlreadySaved()) {
        std::cout << "Image already saved" << std::endl;
        return;
    }

    // Check if we need to remove old images
    if (savedImages.size() >= MAX_SAVED_IMAGES) {
        cleanupOldestImages();
    }
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

    // Show saved notification
    showImageSavedNotification();

    // Invalidate the cache since we just saved the image
    invalidateAlreadySavedCache();

    // Check if gallery is getting full
    checkGalleryFull();
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

    // Load thumbnails for current images
    loadGalleryThumbnails();
}

void ImageGenerator::loadGalleryThumbnails() {
    auto currentImages = getCurrentGalleryImages();

    // Clear existing thumbnails safely
    galleryThumbnails.clear();
    galleryThumbnailSprites.clear();

    // Reserve space to avoid reallocation issues
    galleryThumbnails.reserve(currentImages.size());
    galleryThumbnailSprites.reserve(currentImages.size());

    // Load textures for current gallery images
    for (size_t i = 0; i < currentImages.size(); i++) {
        try {
            sf::Texture thumbnail;
            if (thumbnail.loadFromFile(currentImages[i].filename)) {
                galleryThumbnails.push_back(thumbnail);

                // Create sprite with texture (SFML 3.0 requires texture in constructor)
                sf::Sprite sprite{ galleryThumbnails.back() };

                // Scale to thumbnail size (200x200)
                float scaleX = 200.0f / galleryThumbnails.back().getSize().x;
                float scaleY = 200.0f / galleryThumbnails.back().getSize().y;
                float scale = std::min(scaleX, scaleY);
                sprite.setScale({ scale, scale });

                galleryThumbnailSprites.push_back(sprite);
            }
            else {
                std::cout << "Failed to load thumbnail: " << currentImages[i].filename << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception loading thumbnail " << currentImages[i].filename << ": " << e.what() << std::endl;
        }
    }

    std::cout << "Loaded " << galleryThumbnailSprites.size() << " thumbnails out of " << currentImages.size() << " images" << std::endl;
}

void ImageGenerator::viewSavedImage(const SavedImage& savedImg) {
    // Load the full resolution image
    if (imageTexture.loadFromFile(savedImg.filename)) {
        std::cout << "Viewing saved image: " << savedImg.filename << std::endl;
        imageSprite.setTexture(imageTexture, true);

        // Scale image to fit screen while maintaining aspect ratio
        float scaleX = 1024.0f / imageTexture.getSize().x;
        float scaleY = 768.0f / imageTexture.getSize().y;
        float scale = std::min(scaleX, scaleY);
        imageSprite.setScale({ scale, scale });

        // Center the image
        sf::FloatRect spriteBounds = imageSprite.getGlobalBounds();
        float posX = (1024 - spriteBounds.size.x) / 2;
        float posY = (768 - spriteBounds.size.y) / 2;
        imageSprite.setPosition({ posX, posY });

        // Update button positions based on image orientation
        updateImageDisplayButtonPositions();

        // Store the current viewing image metadata
        currentViewingImage = savedImg;
        viewingFromGallery = true; // CRITICAL: Only set to true when viewing from gallery

        // Switch to image display state
        currentState = AppState::IMAGE_DISPLAY;

        std::cout << "Set viewingFromGallery = true for saved image view" << std::endl;
    }
    else {
        std::cout << "Failed to load saved image: " << savedImg.filename << std::endl;
    }
}

void ImageGenerator::restoreImageMetadata(const SavedImage& savedImg) {
    // Restore the prompt
    userPrompt = savedImg.prompt;
    promptText.setString(userPrompt);
    cursorPosition = userPrompt.length();
    updateCursorPosition();

    // Restore the category (APIModel)
    for (int i = 0; i < modelNames.size(); i++) {
        if (modelNames[i] == savedImg.category) {
            selectedModel = static_cast<APIModel>(i);
            break;
        }
    }

    // Restore the style (StyleMode) - this is more complex since we need to find the enum value
    selectedStyle = StyleMode::NONE; // Default

    // Check all style categories to find matching style name
    for (size_t i = 0; i < artisticStyleNames.size(); i++) {
        if (artisticStyleNames[i] == savedImg.style) {
            selectedStyle = artisticStyles[i];
            break;
        }
    }

    if (selectedStyle == StyleMode::NONE) {
        for (size_t i = 0; i < interiorStyleNames.size(); i++) {
            if (interiorStyleNames[i] == savedImg.style) {
                selectedStyle = interiorStyles[i];
                break;
            }
        }
    }

    if (selectedStyle == StyleMode::NONE) {
        for (size_t i = 0; i < gamingTechStyleNames.size(); i++) {
            if (gamingTechStyleNames[i] == savedImg.style) {
                selectedStyle = gamingTechStyles[i];
                break;
            }
        }
    }

    if (selectedStyle == StyleMode::NONE) {
        for (size_t i = 0; i < entertainmentStyleNames.size(); i++) {
            if (entertainmentStyleNames[i] == savedImg.style) {
                selectedStyle = entertainmentStyles[i];
                break;
            }
        }
    }

    if (selectedStyle == StyleMode::NONE) {
        for (size_t i = 0; i < professionalStyleNames.size(); i++) {
            if (professionalStyleNames[i] == savedImg.style) {
                selectedStyle = professionalStyles[i];
                break;
            }
        }
    }

    if (selectedStyle == StyleMode::NONE) {
        for (size_t i = 0; i < specialtyRoomStyleNames.size(); i++) {
            if (specialtyRoomStyleNames[i] == savedImg.style) {
                selectedStyle = specialtyRoomStyles[i];
                break;
            }
        }
    }

    if (selectedStyle == StyleMode::NONE) {
        for (size_t i = 0; i < landscapeStyleNames.size(); i++) {
            if (landscapeStyleNames[i] == savedImg.style) {
                selectedStyle = landscapeStyles[i];
                break;
            }
        }
    }

    // Handle legacy styles
    if (savedImg.style == "Studio Ghibli") {
        selectedStyle = StyleMode::STUDIO_GHIBLI;
    }
    else if (savedImg.style == "Photorealistic") {
        selectedStyle = StyleMode::PHOTOREALISTIC;
    }

    // Restore orientation
    globalOrientation = savedImg.isLandscape ? OrientationMode::LANDSCAPE : OrientationMode::PORTRAIT;
    orientationLabel.setString(globalOrientation == OrientationMode::PORTRAIT ? "Portrait" : "Landscape");
    sf::FloatRect orientBounds = orientationLabel.getLocalBounds();
    orientationLabel.setPosition({ 532 + (120 - orientBounds.size.x) / 2, 715 });

    // Update UI elements
    updateModelButtons();
    updateStyleButtons();

    if (selectedModel == APIModel::ARTISTIC) {
        updateArtisticButtonPositions();
    }
    else {
        updateCategoryStylePositions();
    }

    // Reset scroll offset
    artisticScrollOffset = 0;

    // Clear viewing state
    viewingFromGallery = false;

    std::cout << "Restored metadata - Category: " << savedImg.category
        << ", Style: " << savedImg.style
        << ", Orientation: " << (savedImg.isLandscape ? "Landscape" : "Portrait") << std::endl;
}

void ImageGenerator::updateLoadingSpinner() {
    if (currentState == AppState::LOADING) {
        // Rotate spinner based on elapsed time
        float elapsed = spinnerClock.getElapsedTime().asSeconds();
        spinnerRotation = elapsed * 360.0f; // One full rotation per second
        loadingSpinner.setRotation(sf::degrees(spinnerRotation));
    }
}

bool ImageGenerator::isImageAlreadySaved() {
    // Return cached result if valid
    if (imageAlreadySavedCacheValid) {
        return imageAlreadySavedCache;
    }

    // Add one-time debug output when cache is being calculated
    std::cout << "Calculating isImageAlreadySaved - currentGeneratedImagePath: '"
        << currentGeneratedImagePath << "'" << std::endl;

    if (currentGeneratedImagePath.empty()) {
        std::cout << "Path is empty, returning false" << std::endl;
        imageAlreadySavedCache = false;
        imageAlreadySavedCacheValid = true;
        return false;
    }

    std::string currentPrompt = userPrompt;
    std::string currentCategory = getCategoryName(selectedModel);
    std::string currentStyle = getStyleName(selectedStyle);
    bool currentIsLandscape = (globalOrientation == OrientationMode::LANDSCAPE);

    std::cout << "Checking: Prompt='" << currentPrompt << "', Category='" << currentCategory
        << "', Style='" << currentStyle << "', Landscape=" << currentIsLandscape << std::endl;

    for (const auto& img : savedImages) {
        if (img.prompt == currentPrompt &&
            img.category == currentCategory &&
            img.style == currentStyle &&
            img.isLandscape == currentIsLandscape) {
            std::cout << "Found matching saved image, returning true" << std::endl;
            imageAlreadySavedCache = true;
            imageAlreadySavedCacheValid = true;
            return true;
        }
    }

    std::cout << "No matching saved image found, returning false" << std::endl;
    imageAlreadySavedCache = false;
    imageAlreadySavedCacheValid = true;
    return false;
}

// Add this new method to invalidate the cache:
void ImageGenerator::invalidateAlreadySavedCache() {
    imageAlreadySavedCacheValid = false;
}

void ImageGenerator::deleteCurrentViewingImage() {
    if (!viewingFromGallery) return;

    // Remove from saved images vector
    auto it = std::find_if(savedImages.begin(), savedImages.end(),
        [this](const SavedImage& img) {
            return img.filename == currentViewingImage.filename;
        });

    if (it != savedImages.end()) {
        // Delete the file
        try {
            std::filesystem::remove(it->filename);
            std::cout << "Deleted image: " << it->filename << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Error deleting image: " << e.what() << std::endl;
        }

        // Remove from vector
        savedImages.erase(it);
        saveSavedImagesMetadata();

        // Return to gallery
        currentState = AppState::GALLERY_SCREEN;
        updateGalleryDisplay();
    }
}

void ImageGenerator::showImageSavedNotification() {
    showImageSavedIndicator = true;
    savedIndicatorClock.restart();
}

void ImageGenerator::checkGalleryFull() {
    if (savedImages.size() >= MAX_SAVED_IMAGES - 2) { // Show warning at 48/50
        showGalleryFullWarning = true;
        warningClock.restart();
    }
}

void ImageGenerator::updateImageDisplayButtonPositions() {
    // Get the current image bounds
    sf::FloatRect imageBounds = imageSprite.getGlobalBounds();

    // Screen dimensions and padding
    const float SCREEN_WIDTH = 1024.0f;
    const float SCREEN_HEIGHT = 768.0f;
    const float PADDING = 20.0f;

    // Determine if image is landscape or portrait based on its dimensions
    bool isLandscapeImage = imageBounds.size.x > imageBounds.size.y;

    if (isLandscapeImage) {
        // LANDSCAPE IMAGE LAYOUT
        // Calculate bottom position with padding from screen bottom
        float bottomY = SCREEN_HEIGHT - 70.0f; // 50px button height + 20px padding

        // New Image button: Bottom right with padding
        newImageButton.setPosition({ SCREEN_WIDTH - 150 - PADDING, bottomY });

        // Reposition the label
        sf::FloatRect newBounds = newImageLabel.getLocalBounds();
        newImageLabel.setPosition({ SCREEN_WIDTH - 150 - PADDING + (150 - newBounds.size.x) / 2,
                                  bottomY + 17 });

        // Other buttons: Bottom left, side by side, with padding
        // Save Image button (left-most)
        saveImageButton.setPosition({ PADDING, bottomY });
        sf::FloatRect saveBounds = saveImageLabel.getLocalBounds();
        saveImageLabel.setPosition({ PADDING + (150 - saveBounds.size.x) / 2,
                                   bottomY + 17 });

        // Gallery button (middle) - when viewing from gallery
        backToGalleryButton.setPosition({ PADDING + 170, bottomY });
        sf::FloatRect gallBounds2 = backToGalleryLabel.getLocalBounds();
        backToGalleryLabel.setPosition({ PADDING + 170 + (120 - gallBounds2.size.x) / 2,
                                       bottomY + 17 });

        // Delete button (right of gallery) - when viewing from gallery  
        deleteImageButton.setPosition({ PADDING + 310, bottomY });
        sf::FloatRect deleteBounds = deleteImageLabel.getLocalBounds();
        deleteImageLabel.setPosition({ PADDING + 310 + (120 - deleteBounds.size.x) / 2,
                                     bottomY + 17 });
    }
    else {
        // PORTRAIT IMAGE LAYOUT
        // New Image button: Bottom right with padding
        float newImageY = SCREEN_HEIGHT - 70.0f; // 50px button height + 20px padding
        newImageButton.setPosition({ SCREEN_WIDTH - 150 - PADDING, newImageY });

        // Reposition the label
        sf::FloatRect newBounds = newImageLabel.getLocalBounds();
        newImageLabel.setPosition({ SCREEN_WIDTH - 150 - PADDING + (150 - newBounds.size.x) / 2,
                                  newImageY + 17 });

        // Other buttons: Bottom left, stacked vertically, with padding
        float leftButtonX = PADDING;

        // Calculate positions from bottom up
        float deleteY = SCREEN_HEIGHT - 70.0f; // Bottom button
        float galleryY = deleteY - 70.0f; // Middle button  
        float saveY = galleryY - 70.0f; // Top button

        // Save Image button (bottom of stack)
        saveImageButton.setPosition({ leftButtonX, deleteY });
        sf::FloatRect saveBounds = saveImageLabel.getLocalBounds();
        saveImageLabel.setPosition({ leftButtonX + (150 - saveBounds.size.x) / 2,
                                   deleteY + 17 });

        // Gallery button (middle) - when viewing from gallery
        backToGalleryButton.setPosition({ leftButtonX, galleryY });
        sf::FloatRect gallBounds2 = backToGalleryLabel.getLocalBounds();
        backToGalleryLabel.setPosition({ leftButtonX + (120 - gallBounds2.size.x) / 2,
                                       galleryY + 17 });

        // Delete button (top of stack) - when viewing from gallery
        deleteImageButton.setPosition({ leftButtonX, saveY });
        sf::FloatRect deleteBounds = deleteImageLabel.getLocalBounds();
        deleteImageLabel.setPosition({ leftButtonX + (120 - deleteBounds.size.x) / 2,
                                     saveY + 17 });
    }

    std::cout << "Updated button positions for " << (isLandscapeImage ? "landscape" : "portrait") << " image" << std::endl;
}
#include "ImageGenerator.h"

void ImageGenerator::initializeUI() {
    // Prompt label
    promptLabel.setFont(font);
    promptLabel.setString("Enter your image prompt:");
    promptLabel.setCharacterSize(24);
    promptLabel.setFillColor(sf::Color::White);
    promptLabel.setPosition({ 50, 50 });

    // Prompt input box
    promptBox.setSize({ 924, 50 });
    promptBox.setPosition({ 50, 90 });
    promptBox.setFillColor(sf::Color(60, 60, 60));
    promptBox.setOutlineThickness(2);
    promptBox.setOutlineColor(sf::Color(100, 100, 100));

    // Prompt text
    promptText.setFont(font);
    promptText.setCharacterSize(20);
    promptText.setFillColor(sf::Color::White);
    promptText.setPosition({ 60, 105 });

    // Model selection buttons (Now 8 categories) - Two rows of 4
    for (int i = 0; i < 8; i++) {
        sf::RectangleShape button({ 120, 40 }); // Smaller buttons to fit 8
        int row = i / 4;
        int col = i % 4;
        button.setPosition({ 50 + col * 130.0f, 160 + row * 50.0f });
        button.setFillColor(i == 0 ? selectedButtonColor : buttonColor);
        modelButtons.push_back(button);

        sf::Text label(font, "");
        label.setFont(font);
        label.setString(modelNames[i]);
        label.setCharacterSize(14); // Smaller font for 8 buttons
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 50 + col * 130.0f + (120 - textBounds.size.x) / 2,
                        160 + row * 50.0f + (40 - textBounds.size.y) / 2 - 3 });
        modelLabels.push_back(label);
    }

    // Styles header for Realism/Aesthetic models (legacy)
    stylesGroupLabel.setFont(font);
    stylesGroupLabel.setString("Styles");
    stylesGroupLabel.setCharacterSize(20);
    stylesGroupLabel.setFillColor(sf::Color::White);
    stylesGroupLabel.setPosition({ 50, 270 }); // Moved down to accommodate 2 rows of model buttons

    // Style buttons - Studio Ghibli and Photorealistic (for Realism/Aesthetic models)
    ghibliButton.setSize({ 220, 40 });
    ghibliButton.setPosition({ 50, 300 }); // Moved down
    ghibliButton.setFillColor(buttonColor);

    ghibliLabel.setFont(font);
    ghibliLabel.setString("Studio Ghibli");
    ghibliLabel.setCharacterSize(14);
    ghibliLabel.setFillColor(sf::Color::White);
    sf::FloatRect ghibliBounds = ghibliLabel.getLocalBounds();
    ghibliLabel.setPosition({ 50 + (220 - ghibliBounds.size.x) / 2,
                    300 + (40 - ghibliBounds.size.y) / 2 - 3 });

    photorealisticButton.setSize({ 220, 40 });
    photorealisticButton.setPosition({ 290, 300 }); // Moved down
    photorealisticButton.setFillColor(buttonColor);

    photorealisticLabel.setFont(font);
    photorealisticLabel.setString("Photorealistic");
    photorealisticLabel.setCharacterSize(14);
    photorealisticLabel.setFillColor(sf::Color::White);
    sf::FloatRect photoBounds = photorealisticLabel.getLocalBounds();
    photorealisticLabel.setPosition({ 290 + (220 - photoBounds.size.x) / 2,
                    300 + (40 - photoBounds.size.y) / 2 - 3 });

    // Category styles header (for new categories)
    categoryGroupLabel.setFont(font);
    categoryGroupLabel.setString("Styles");
    categoryGroupLabel.setCharacterSize(20);
    categoryGroupLabel.setFillColor(sf::Color::White);
    categoryGroupLabel.setPosition({ 50, 270 }); // Same position as stylesGroupLabel

    // Group labels for artistic styles (existing)
    artisticGroupLabel.setFont(font);
    artisticGroupLabel.setString("Artistic Styles");
    artisticGroupLabel.setCharacterSize(20);
    artisticGroupLabel.setFillColor(sf::Color::White);
    artisticGroupLabel.setPosition({ 50, 270 }); // Moved down

    // Define scrollable area for all categories
    artisticScrollArea.setSize({ 924, 350 }); // Reduced height due to 2 model button rows
    artisticScrollArea.setPosition({ 50, 270 });

    // Initialize artistic style buttons (4 columns, existing)
    for (size_t i = 0; i < artisticStyles.size(); i++) {
        int col = i % 4;
        int row = i / 4;

        sf::RectangleShape button({ 220, 40 });
        button.setPosition({ 50 + col * 240.0f, 260 + row * 50.0f });
        button.setFillColor(buttonColor);
        artisticStyleButtons.push_back(button);

        sf::Text label(font);
        label.setString(artisticStyleNames[i]);
        label.setCharacterSize(14);
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                        260 + row * 50.0f + (40 - textBounds.size.y) / 2 - 3 });
        artisticStyleLabels.push_back(label);
    }

    // Initialize interior design style buttons (existing)
    int artisticRows = (artisticStyles.size() + 3) / 4;
    float interiorStartY = 310 + artisticRows * 50.0f;

    interiorGroupLabel.setFont(font);
    interiorGroupLabel.setString("Interior Design Styles");
    interiorGroupLabel.setCharacterSize(20);
    interiorGroupLabel.setFillColor(sf::Color::White);
    interiorGroupLabel.setPosition({ 50, interiorStartY - 40 });

    for (size_t i = 0; i < interiorStyles.size(); i++) {
        int col = i % 4;
        int row = i / 4;

        sf::RectangleShape button({ 220, 40 });
        button.setPosition({ 50 + col * 240.0f, interiorStartY + row * 50.0f });
        button.setFillColor(buttonColor);
        interiorStyleButtons.push_back(button);

        sf::Text label(font);
        label.setString(interiorStyleNames[i]);
        label.setCharacterSize(14);
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                        interiorStartY + row * 50.0f + (40 - textBounds.size.y) / 2 - 3 });
        interiorStyleLabels.push_back(label);
    }

    // Initialize category style buttons (for new categories)
    // These will be dynamically populated based on selected category

    // Generate button - positioned at bottom
    generateButton.setSize({ 200, 50 });
    generateButton.setPosition({ 312, 700 }); // Moved left to make room for other buttons
    generateButton.setFillColor(sf::Color(50, 150, 50));

    generateLabel.setFont(font);
    generateLabel.setString("Generate Image");
    generateLabel.setCharacterSize(18);
    generateLabel.setFillColor(sf::Color::White);
    sf::FloatRect genBounds = generateLabel.getLocalBounds();
    generateLabel.setPosition({ 312 + (200 - genBounds.size.x) / 2, 715 });

    // Orientation toggle button
    orientationButton.setSize({ 120, 50 });
    orientationButton.setPosition({ 532, 700 }); // Next to generate button
    orientationButton.setFillColor(buttonColor);

    orientationLabel.setFont(font);
    orientationLabel.setString("Portrait");
    orientationLabel.setCharacterSize(16);
    orientationLabel.setFillColor(sf::Color::White);
    sf::FloatRect orientBounds = orientationLabel.getLocalBounds();
    orientationLabel.setPosition({ 532 + (120 - orientBounds.size.x) / 2, 715 });

    // Gallery button
    galleryButton.setSize({ 120, 50 });
    galleryButton.setPosition({ 672, 700 }); // Next to orientation button
    galleryButton.setFillColor(sf::Color(100, 100, 150));

    galleryLabel.setFont(font);
    galleryLabel.setString("Gallery");
    galleryLabel.setCharacterSize(16);
    galleryLabel.setFillColor(sf::Color::White);
    sf::FloatRect gallBounds = galleryLabel.getLocalBounds();
    galleryLabel.setPosition({ 672 + (120 - gallBounds.size.x) / 2, 715 });

    // Save image button (for image display screen)
    saveImageButton.setSize({ 150, 50 });
    saveImageButton.setPosition({ 600, 648 });
    saveImageButton.setFillColor(sf::Color(150, 100, 50));

    saveImageLabel.setFont(font);
    saveImageLabel.setString("Save Image");
    saveImageLabel.setCharacterSize(16);
    saveImageLabel.setFillColor(sf::Color::White);
    sf::FloatRect saveBounds = saveImageLabel.getLocalBounds();
    saveImageLabel.setPosition({ 600 + (150 - saveBounds.size.x) / 2, 665 });

    // New image button (for image display screen) - repositioned
    newImageButton.setSize({ 150, 50 });
    newImageButton.setPosition({ 774, 648 });
    newImageButton.setFillColor(sf::Color(50, 150, 50));

    newImageLabel.setFont(font);
    newImageLabel.setString("New Image");
    newImageLabel.setCharacterSize(16);
    newImageLabel.setFillColor(sf::Color::White);
    sf::FloatRect newBounds = newImageLabel.getLocalBounds();
    newImageLabel.setPosition({ 774 + (150 - newBounds.size.x) / 2, 665 });

    // Gallery screen UI elements
    backToMainButton.setSize({ 120, 40 });
    backToMainButton.setPosition({ 50, 50 });
    backToMainButton.setFillColor(buttonColor);

    backToMainLabel.setFont(font);
    backToMainLabel.setString("Back");
    backToMainLabel.setCharacterSize(16);
    backToMainLabel.setFillColor(sf::Color::White);
    sf::FloatRect backBounds = backToMainLabel.getLocalBounds();
    backToMainLabel.setPosition({ 50 + (120 - backBounds.size.x) / 2, 60 });

    // Gallery header
    galleryHeaderLabel.setFont(font);
    galleryHeaderLabel.setString("Saved Images Gallery");
    galleryHeaderLabel.setCharacterSize(28);
    galleryHeaderLabel.setFillColor(sf::Color::White);
    sf::FloatRect headerBounds = galleryHeaderLabel.getLocalBounds();
    galleryHeaderLabel.setPosition({ (1024 - headerBounds.size.x) / 2, 50 });

    // Gallery orientation tabs
    portraitTabButton.setSize({ 100, 40 });
    portraitTabButton.setPosition({ 400, 120 });
    portraitTabButton.setFillColor(selectedButtonColor);

    portraitTabLabel.setFont(font);
    portraitTabLabel.setString("Portrait");
    portraitTabLabel.setCharacterSize(14);
    portraitTabLabel.setFillColor(sf::Color::White);
    sf::FloatRect portBounds = portraitTabLabel.getLocalBounds();
    portraitTabLabel.setPosition({ 400 + (100 - portBounds.size.x) / 2, 130 });

    landscapeTabButton.setSize({ 100, 40 });
    landscapeTabButton.setPosition({ 524, 120 });
    landscapeTabButton.setFillColor(buttonColor);

    landscapeTabLabel.setFont(font);
    landscapeTabLabel.setString("Landscape");
    landscapeTabLabel.setCharacterSize(14);
    landscapeTabLabel.setFillColor(sf::Color::White);
    sf::FloatRect landBounds = landscapeTabLabel.getLocalBounds();
    landscapeTabLabel.setPosition({ 524 + (100 - landBounds.size.x) / 2, 130 });

    // Gallery info label
    galleryInfoLabel.setFont(font);
    galleryInfoLabel.setString("No saved images");
    galleryInfoLabel.setCharacterSize(16);
    galleryInfoLabel.setFillColor(sf::Color(150, 150, 150));
    sf::FloatRect infoBounds = galleryInfoLabel.getLocalBounds();
    galleryInfoLabel.setPosition({ (1024 - infoBounds.size.x) / 2, 300 });

    // Gallery scroll area
    galleryScrollArea.setSize({ 924, 400 });
    galleryScrollArea.setPosition({ 50, 180 });

    // Loading text
    loadingText.setFont(font);
    loadingText.setString("Generating image...");
    loadingText.setCharacterSize(32);
    loadingText.setFillColor(sf::Color::White);
    sf::FloatRect loadBounds = loadingText.getLocalBounds();
    loadingText.setPosition({ (1024 - loadBounds.size.x) / 2,
                           (768 - loadBounds.size.y) / 2 });

    // Initialize current category styles as empty
    currentCategoryStyles.clear();
    currentCategoryStyleNames.clear();
}

void ImageGenerator::updateCategoryStylePositions() {
    // Clear existing category style buttons
    categoryStyleButtons.clear();
    categoryStyleLabels.clear();

    // Get the appropriate styles for the current model
    std::vector<StyleMode>* styles = nullptr;
    std::vector<std::string>* styleNames = nullptr;

    switch (selectedModel) {
    case APIModel::GAMING_TECH:
        styles = &gamingTechStyles;
        styleNames = &gamingTechStyleNames;
        break;
    case APIModel::ENTERTAINMENT:
        styles = &entertainmentStyles;
        styleNames = &entertainmentStyleNames;
        break;
    case APIModel::PROFESSIONAL:
        styles = &professionalStyles;
        styleNames = &professionalStyleNames;
        break;
    case APIModel::SPECIALTY_ROOMS:
        styles = &specialtyRoomStyles;
        styleNames = &specialtyRoomStyleNames;
        break;
    case APIModel::LANDSCAPES:
        styles = &landscapeStyles;
        styleNames = &landscapeStyleNames;
        break;
    default:
        return; // No category styles to show
    }

    if (!styles || !styleNames) return;

    // Update current category for click handling
    currentCategoryStyles = *styles;
    currentCategoryStyleNames = *styleNames;

    // Create buttons for current category styles
    for (size_t i = 0; i < styles->size(); i++) {
        int col = i % 4;
        int row = i / 4;

        sf::RectangleShape button({ 220, 40 });
        float baseY = 300 + row * 50.0f + artisticScrollOffset;
        button.setPosition({ 50 + col * 240.0f, baseY });

        // Set button color based on selection
        if (selectedStyle == (*styles)[i]) {
            button.setFillColor(selectedButtonColor);
        }
        else {
            button.setFillColor(buttonColor);
        }
        categoryStyleButtons.push_back(button);

        sf::Text label(font);
        label.setString((*styleNames)[i]);
        label.setCharacterSize(14);
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                        baseY + (40 - textBounds.size.y) / 2 - 3 });
        categoryStyleLabels.push_back(label);
    }
}

void ImageGenerator::updateStyleButtons() {
    // Update Studio Ghibli button
    if (selectedStyle == StyleMode::STUDIO_GHIBLI) {
        ghibliButton.setFillColor(selectedButtonColor);
    }
    else {
        ghibliButton.setFillColor(buttonColor);
    }

    // Update Photorealistic button
    if (selectedStyle == StyleMode::PHOTOREALISTIC) {
        photorealisticButton.setFillColor(selectedButtonColor);
    }
    else {
        photorealisticButton.setFillColor(buttonColor);
    }

    // Update artistic style buttons
    for (size_t i = 0; i < artisticStyleButtons.size(); i++) {
        if (selectedStyle == artisticStyles[i]) {
            artisticStyleButtons[i].setFillColor(selectedButtonColor);
        }
        else {
            artisticStyleButtons[i].setFillColor(buttonColor);
        }
    }

    // Update interior style buttons
    for (size_t i = 0; i < interiorStyleButtons.size(); i++) {
        if (selectedStyle == interiorStyles[i]) {
            interiorStyleButtons[i].setFillColor(selectedButtonColor);
        }
        else {
            interiorStyleButtons[i].setFillColor(buttonColor);
        }
    }

    // Update category style buttons (for new categories)
    for (size_t i = 0; i < categoryStyleButtons.size(); i++) {
        if (i < currentCategoryStyles.size() && selectedStyle == currentCategoryStyles[i]) {
            categoryStyleButtons[i].setFillColor(selectedButtonColor);
        }
        else {
            categoryStyleButtons[i].setFillColor(buttonColor);
        }
    }

    // Update category style positions when styles change
    if (selectedModel == APIModel::GAMING_TECH ||
        selectedModel == APIModel::ENTERTAINMENT ||
        selectedModel == APIModel::PROFESSIONAL ||
        selectedModel == APIModel::SPECIALTY_ROOMS ||
        selectedModel == APIModel::LANDSCAPES) {
        updateCategoryStylePositions();
    }
}

void ImageGenerator::updateModelButtons() {
    for (int i = 0; i < modelButtons.size(); i++) {
        if (i == static_cast<int>(selectedModel)) {
            modelButtons[i].setFillColor(selectedButtonColor);
        }
        else if (i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC) {
            // Disable Aesthetic button when Photorealistic is selected
            modelButtons[i].setFillColor(disabledButtonColor);
        }
        else {
            modelButtons[i].setFillColor(buttonColor);
        }
    }
}

void ImageGenerator::updateButtonHovers(sf::Vector2f mousePos) {
    // Generate button hover
    if (generateButton.getGlobalBounds().contains(mousePos)) {
        generateButton.setFillColor(sf::Color(70, 170, 70));
    }
    else {
        generateButton.setFillColor(sf::Color(50, 150, 50));
    }

    // Orientation button hover
    if (orientationButton.getGlobalBounds().contains(mousePos)) {
        orientationButton.setFillColor(buttonHoverColor);
    }
    else {
        orientationButton.setFillColor(buttonColor);
    }

    // Gallery button hover
    if (galleryButton.getGlobalBounds().contains(mousePos)) {
        galleryButton.setFillColor(sf::Color(120, 120, 170));
    }
    else {
        galleryButton.setFillColor(sf::Color(100, 100, 150));
    }

    // Show hover effects based on current model
    if (selectedModel == APIModel::REALISM || selectedModel == APIModel::AESTHETIC) {
        // Studio Ghibli button hover
        if (ghibliButton.getGlobalBounds().contains(mousePos)) {
            if (selectedStyle != StyleMode::STUDIO_GHIBLI) {
                ghibliButton.setFillColor(buttonHoverColor);
            }
        }
        else {
            if (selectedStyle != StyleMode::STUDIO_GHIBLI) {
                ghibliButton.setFillColor(buttonColor);
            }
        }

        // Photorealistic button hover
        if (photorealisticButton.getGlobalBounds().contains(mousePos)) {
            if (selectedStyle != StyleMode::PHOTOREALISTIC) {
                photorealisticButton.setFillColor(buttonHoverColor);
            }
        }
        else {
            if (selectedStyle != StyleMode::PHOTOREALISTIC) {
                photorealisticButton.setFillColor(buttonColor);
            }
        }
    }
    else if (selectedModel == APIModel::ARTISTIC) {
        // Artistic style button hovers
        for (size_t i = 0; i < artisticStyleButtons.size(); i++) {
            if (artisticStyleButtons[i].getGlobalBounds().contains(mousePos)) {
                if (selectedStyle != artisticStyles[i]) {
                    artisticStyleButtons[i].setFillColor(buttonHoverColor);
                }
            }
            else {
                if (selectedStyle != artisticStyles[i]) {
                    artisticStyleButtons[i].setFillColor(buttonColor);
                }
            }
        }

        // Interior style button hovers
        for (size_t i = 0; i < interiorStyleButtons.size(); i++) {
            if (interiorStyleButtons[i].getGlobalBounds().contains(mousePos)) {
                if (selectedStyle != interiorStyles[i]) {
                    interiorStyleButtons[i].setFillColor(buttonHoverColor);
                }
            }
            else {
                if (selectedStyle != interiorStyles[i]) {
                    interiorStyleButtons[i].setFillColor(buttonColor);
                }
            }
        }
    }
    else {
        // Category style button hovers (for new categories)
        for (size_t i = 0; i < categoryStyleButtons.size(); i++) {
            if (categoryStyleButtons[i].getGlobalBounds().contains(mousePos)) {
                if (i < currentCategoryStyles.size() && selectedStyle != currentCategoryStyles[i]) {
                    categoryStyleButtons[i].setFillColor(buttonHoverColor);
                }
            }
            else {
                if (i < currentCategoryStyles.size() && selectedStyle != currentCategoryStyles[i]) {
                    categoryStyleButtons[i].setFillColor(buttonColor);
                }
            }
        }
    }

    // Model button hovers
    for (int i = 0; i < modelButtons.size(); i++) {
        if (modelButtons[i].getGlobalBounds().contains(mousePos)) {
            if (i != static_cast<int>(selectedModel) &&
                !(i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC)) {
                modelButtons[i].setFillColor(buttonHoverColor);
            }
        }
        else {
            if (i != static_cast<int>(selectedModel)) {
                if (i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC) {
                    modelButtons[i].setFillColor(disabledButtonColor);
                }
                else {
                    modelButtons[i].setFillColor(buttonColor);
                }
            }
        }
    }
}

void ImageGenerator::updateArtisticButtonPositions() {
    // Update artistic style button positions based on scroll offset
    for (size_t i = 0; i < artisticStyleButtons.size(); i++) {
        int col = i % 4;
        int row = i / 4;

        float baseY = 260 + row * 50.0f + artisticScrollOffset;
        artisticStyleButtons[i].setPosition({ 50 + col * 240.0f, baseY });

        sf::FloatRect textBounds = artisticStyleLabels[i].getLocalBounds();
        artisticStyleLabels[i].setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                                          baseY + (40 - textBounds.size.y) / 2 - 3 });
    }

    // Update interior style button positions based on scroll offset  
    int artisticRows = (artisticStyles.size() + 3) / 4;
    float interiorStartY = 310 + artisticRows * 50.0f;

    for (size_t i = 0; i < interiorStyleButtons.size(); i++) {
        int col = i % 4;
        int row = i / 4;

        float baseY = interiorStartY + row * 50.0f + artisticScrollOffset;
        interiorStyleButtons[i].setPosition({ 50 + col * 240.0f, baseY });

        sf::FloatRect textBounds = interiorStyleLabels[i].getLocalBounds();
        interiorStyleLabels[i].setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                                          baseY + (40 - textBounds.size.y) / 2 - 3 });
    }

    // Update both group labels to scroll with content
    artisticGroupLabel.setPosition({ 50, 270 + artisticScrollOffset });
    interiorGroupLabel.setPosition({ 50, interiorStartY - 40 + artisticScrollOffset });
}

void ImageGenerator::updateCursorPosition() {
    sf::Text tempText(font, "");
    tempText.setFont(font);
    tempText.setCharacterSize(20);
    tempText.setString(userPrompt.substr(0, cursorPosition));

    float cursorX = promptBox.getPosition().x + 10 + tempText.getLocalBounds().size.x;
    cursor.setPosition({ cursorX, promptBox.getPosition().y + 15 });
}

void ImageGenerator::render() {
    window.clear(backgroundColor);

    // Handle cursor blinking
    if (promptActive && cursorClock.getElapsedTime().asMilliseconds() > 500) {
        cursorVisible = !cursorVisible;
        cursorClock.restart();
    }

    switch (currentState) {
    case AppState::INPUT_SCREEN:
        renderInputScreen();
        break;
    case AppState::LOADING:
        renderLoadingScreen();
        break;
    case AppState::IMAGE_DISPLAY:
        renderImageDisplay();
        break;
    case AppState::GALLERY_SCREEN:
        renderGalleryScreen();
        break;
    }

    window.display();
}

void ImageGenerator::renderInputScreen() {
    window.draw(promptLabel);
    window.draw(promptBox);
    window.draw(promptText);

    // Draw cursor if textbox is active and cursor is visible
    if (promptActive && cursorVisible) {
        window.draw(cursor);
    }

    // Draw model buttons (now 8 categories in 2 rows)
    for (const auto& button : modelButtons) {
        window.draw(button);
    }
    for (const auto& label : modelLabels) {
        window.draw(label);
    }

    // Draw appropriate style buttons based on selected model
    if (selectedModel == APIModel::ARTISTIC) {
        // Draw artistic styles with STRICT clipping bounds
        sf::Vector2f artisticLabelPos = artisticGroupLabel.getPosition();
        if (artisticLabelPos.y >= 270 && artisticLabelPos.y <= 620) {
            window.draw(artisticGroupLabel);
        }

        // Draw artistic style buttons (with strict bounds checking)
        for (size_t i = 0; i < artisticStyleButtons.size(); i++) {
            sf::Vector2f pos = artisticStyleButtons[i].getPosition();
            if (pos.y >= 270 && pos.y <= 620) {
                window.draw(artisticStyleButtons[i]);
                window.draw(artisticStyleLabels[i]);
            }
        }

        // Draw interior group label if visible
        sf::Vector2f labelPos = interiorGroupLabel.getPosition();
        if (labelPos.y >= 270 && labelPos.y <= 620) {
            window.draw(interiorGroupLabel);
        }

        // Draw interior design style buttons (with strict bounds checking)
        for (size_t i = 0; i < interiorStyleButtons.size(); i++) {
            sf::Vector2f pos = interiorStyleButtons[i].getPosition();
            if (pos.y >= 270 && pos.y <= 620) {
                window.draw(interiorStyleButtons[i]);
                window.draw(interiorStyleLabels[i]);
            }
        }
    }
    else if (selectedModel == APIModel::REALISM || selectedModel == APIModel::AESTHETIC) {
        // Draw traditional styles header and buttons (Studio Ghibli, Photorealistic)
        window.draw(stylesGroupLabel);
        window.draw(ghibliButton);
        window.draw(ghibliLabel);
        window.draw(photorealisticButton);
        window.draw(photorealisticLabel);
    }
    else {
        // Draw new category styles
        window.draw(categoryGroupLabel);

        // Draw category style buttons with bounds checking
        for (size_t i = 0; i < categoryStyleButtons.size(); i++) {
            sf::Vector2f pos = categoryStyleButtons[i].getPosition();
            if (pos.y >= 270 && pos.y <= 620) {
                window.draw(categoryStyleButtons[i]);
                window.draw(categoryStyleLabels[i]);
            }
        }
    }

    window.draw(generateButton);
    window.draw(generateLabel);
    window.draw(orientationButton);
    window.draw(orientationLabel);
    window.draw(galleryButton);
    window.draw(galleryLabel);
}

void ImageGenerator::renderLoadingScreen() {
    window.draw(loadingText);
}

void ImageGenerator::renderImageDisplay() {
    window.draw(imageSprite);
    window.draw(saveImageButton);
    window.draw(saveImageLabel);
    window.draw(newImageButton);
    window.draw(newImageLabel);
}

void ImageGenerator::renderGalleryScreen() {
    // Draw header
    window.draw(backToMainButton);
    window.draw(backToMainLabel);
    window.draw(galleryHeaderLabel);

    // Draw tabs
    window.draw(portraitTabButton);
    window.draw(portraitTabLabel);
    window.draw(landscapeTabButton);
    window.draw(landscapeTabLabel);

    // Get current images
    auto currentImages = getCurrentGalleryImages();

    if (currentImages.empty()) {
        window.draw(galleryInfoLabel);
        return;
    }

    // Draw image thumbnails in a grid
    int imagesPerRow = 4;
    float thumbnailSize = 200.0f;
    float spacing = 30.0f;
    float startX = 50.0f;
    float startY = 200.0f - galleryScrollOffset;

    for (size_t i = 0; i < currentImages.size(); i++) {
        int row = i / imagesPerRow;
        int col = i % imagesPerRow;

        float x = startX + col * (thumbnailSize + spacing);
        float y = startY + row * (thumbnailSize + spacing);

        // Only draw if visible in scroll area
        if (y + thumbnailSize >= 180 && y <= 580) {
            // Draw thumbnail placeholder (actual thumbnail loading would be implemented later)
            sf::RectangleShape thumbnail({ thumbnailSize, thumbnailSize });
            thumbnail.setPosition({ x, y });
            thumbnail.setFillColor(sf::Color(80, 80, 80));
            thumbnail.setOutlineThickness(2);
            thumbnail.setOutlineColor(sf::Color(120, 120, 120));
            window.draw(thumbnail);

            // Draw image info text
            sf::Text infoText(font);
            infoText.setString(currentImages[i].category + "\n" + currentImages[i].style);
            infoText.setCharacterSize(12);
            infoText.setFillColor(sf::Color::White);
            infoText.setPosition({ x + 5, y + 5 });
            window.draw(infoText);

            // Draw timestamp
            sf::Text timeText(font);
            timeText.setString(currentImages[i].timestamp.substr(0, 10)); // Just the date part
            timeText.setCharacterSize(10);
            timeText.setFillColor(sf::Color(200, 200, 200));
            timeText.setPosition({ x + 5, y + thumbnailSize - 20 });
            window.draw(timeText);
        }
    }

    // Draw scroll indicator if needed
    if (currentImages.size() > 8) { // More than 2 rows
        sf::Text scrollText(font);
        scrollText.setString("Scroll to see more images");
        scrollText.setCharacterSize(14);
        scrollText.setFillColor(sf::Color(150, 150, 150));
        sf::FloatRect scrollBounds = scrollText.getLocalBounds();
        scrollText.setPosition({ (1024 - scrollBounds.size.x) / 2, 600 });
        window.draw(scrollText);
    }
}
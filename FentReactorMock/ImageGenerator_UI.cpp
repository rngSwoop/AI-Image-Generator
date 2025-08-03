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

    // Model selection buttons (Realism, Aesthetic, Artistic)
    for (int i = 0; i < 3; i++) {
        sf::RectangleShape button({ 150, 50 });
        button.setPosition({ 225 + i * 170.0f, 160 }); // Changed from 140 to 160 (20px more space)
        button.setFillColor(i == 0 ? selectedButtonColor : buttonColor);
        modelButtons.push_back(button);

        sf::Text label(font, "");
        label.setFont(font);
        label.setString(modelNames[i]);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 225 + i * 170.0f + (150 - textBounds.size.x) / 2,
                        160 + (50 - textBounds.size.y) / 2 - 5 }); // Changed from 140 to 160
        modelLabels.push_back(label);
    }

    // Styles header for Realism/Aesthetic models
    stylesGroupLabel.setFont(font);
    stylesGroupLabel.setString("Styles");
    stylesGroupLabel.setCharacterSize(20);
    stylesGroupLabel.setFillColor(sf::Color::White);
    stylesGroupLabel.setPosition({ 50, 240 }); // Same as artistic header position

    // Style buttons - Studio Ghibli and Photorealistic (for Realism/Aesthetic models)
    // Position them like the artistic buttons but in 2 columns
    ghibliButton.setSize({ 220, 40 }); // Match artistic button size
    ghibliButton.setPosition({ 50, 270 }); // First column position
    ghibliButton.setFillColor(buttonColor);

    ghibliLabel.setFont(font);
    ghibliLabel.setString("Studio Ghibli");
    ghibliLabel.setCharacterSize(14); // Match artistic button text size
    ghibliLabel.setFillColor(sf::Color::White);
    sf::FloatRect ghibliBounds = ghibliLabel.getLocalBounds();
    ghibliLabel.setPosition({ 50 + (220 - ghibliBounds.size.x) / 2,
                    270 + (40 - ghibliBounds.size.y) / 2 - 3 }); // Match artistic button centering

    photorealisticButton.setSize({ 220, 40 }); // Match artistic button size
    photorealisticButton.setPosition({ 290, 270 }); // Second column position (50 + 220 + 20 spacing)
    photorealisticButton.setFillColor(buttonColor);

    photorealisticLabel.setFont(font);
    photorealisticLabel.setString("Photorealistic");
    photorealisticLabel.setCharacterSize(14); // Match artistic button text size
    photorealisticLabel.setFillColor(sf::Color::White);
    sf::FloatRect photoBounds = photorealisticLabel.getLocalBounds();
    photorealisticLabel.setPosition({ 290 + (220 - photoBounds.size.x) / 2,
                    270 + (40 - photoBounds.size.y) / 2 - 3 }); // Match artistic button centering

    // Group labels for artistic styles
    artisticGroupLabel.setFont(font);
    artisticGroupLabel.setString("Artistic Styles");
    artisticGroupLabel.setCharacterSize(20);
    artisticGroupLabel.setFillColor(sf::Color::White);
    artisticGroupLabel.setPosition({ 50, 240 }); // Changed from 200 to 240 to match scroll area

    // Define scrollable area for artistic styles - START LOWER to avoid model buttons
    artisticScrollArea.setSize({ 924, 380 }); // Reduced height
    artisticScrollArea.setPosition({ 50, 240 }); // Start below model buttons

    // Initialize artistic style buttons (4 columns, better spacing)
    for (size_t i = 0; i < artisticStyles.size(); i++) {
        int col = i % 4;
        int row = i / 4;

        sf::RectangleShape button({ 220, 40 });  // Changed from 30 to 40 height
        button.setPosition({ 50 + col * 240.0f, 230 + row * 50.0f });  // Changed spacing
        button.setFillColor(buttonColor);
        artisticStyleButtons.push_back(button);

        sf::Text label(font);
        label.setString(artisticStyleNames[i]);
        label.setCharacterSize(14);  // Changed from 12 to 14
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                        230 + row * 50.0f + (40 - textBounds.size.y) / 2 - 3 });
        artisticStyleLabels.push_back(label);
    }

    // Initialize interior design style buttons (4 columns, better spacing)
    int artisticRows = (artisticStyles.size() + 3) / 4; // Round up
    float interiorStartY = 280 + artisticRows * 50.0f; // Start after artistic styles

    // Group labels for interior styles
    interiorGroupLabel.setFont(font);
    interiorGroupLabel.setString("Interior Design Styles");
    interiorGroupLabel.setCharacterSize(20);
    interiorGroupLabel.setFillColor(sf::Color::White);
    // Update interior group label position
    interiorGroupLabel.setPosition({ 50, interiorStartY - 40 }); // Change -30 to -50

    for (size_t i = 0; i < interiorStyles.size(); i++) {
        int col = i % 4;
        int row = i / 4;

        sf::RectangleShape button({ 220, 40 });  // Changed from 30 to 40 height
        button.setPosition({ 50 + col * 240.0f, interiorStartY + row * 50.0f });
        button.setFillColor(buttonColor);
        interiorStyleButtons.push_back(button);

        sf::Text label(font);
        label.setString(interiorStyleNames[i]);
        label.setCharacterSize(14);  // Changed from 12 to 14
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                        interiorStartY + row * 50.0f + (40 - textBounds.size.y) / 2 - 3 });
        interiorStyleLabels.push_back(label);
    }

    // Generate button - positioned at bottom
    generateButton.setSize({ 200, 50 });
    generateButton.setPosition({ 412, 700 });
    generateButton.setFillColor(sf::Color(50, 150, 50));

    generateLabel.setFont(font);
    generateLabel.setString("Generate Image");
    generateLabel.setCharacterSize(18);
    generateLabel.setFillColor(sf::Color::White);
    sf::FloatRect genBounds = generateLabel.getLocalBounds();
    generateLabel.setPosition({ 412 + (200 - genBounds.size.x) / 2, 715 });

    // New image button (for image display screen)
    newImageButton.setSize({ 200, 60 });
    newImageButton.setPosition({ 774, 648 });
    newImageButton.setFillColor(sf::Color(50, 150, 50));

    newImageLabel.setFont(font);
    newImageLabel.setString("New Image");
    newImageLabel.setCharacterSize(20);
    newImageLabel.setFillColor(sf::Color::White);
    sf::FloatRect newBounds = newImageLabel.getLocalBounds();
    newImageLabel.setPosition({ 774 + (200 - newBounds.size.x) / 2, 665 });

    // Loading text
    loadingText.setFont(font);
    loadingText.setString("Generating image...");
    loadingText.setCharacterSize(32);
    loadingText.setFillColor(sf::Color::White);
    sf::FloatRect loadBounds = loadingText.getLocalBounds();
    loadingText.setPosition({ (1024 - loadBounds.size.x) / 2,
                           (768 - loadBounds.size.y) / 2 });
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

    // Only show hover effects for visible buttons
    if (selectedModel != APIModel::ARTISTIC) {
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
    else {
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

        float baseY = 230 + row * 50.0f + artisticScrollOffset;  // Changed from 35.0f to 50.0f
        artisticStyleButtons[i].setPosition({ 50 + col * 240.0f, baseY });  // Changed from 230.0f to 240.0f

        sf::FloatRect textBounds = artisticStyleLabels[i].getLocalBounds();
        artisticStyleLabels[i].setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                                          baseY + (40 - textBounds.size.y) / 2 - 3 });  // Changed from (30-y)/2-2 to (40-y)/2-3
    }

    // Update interior style button positions based on scroll offset  
    int artisticRows = (artisticStyles.size() + 3) / 4;  // Calculate rows properly
    float interiorStartY = 280 + artisticRows * 50.0f;  // Dynamic positioning

    for (size_t i = 0; i < interiorStyleButtons.size(); i++) {
        int col = i % 4;
        int row = i / 4;

        float baseY = interiorStartY + row * 50.0f + artisticScrollOffset;  // Use dynamic start + 50px spacing
        interiorStyleButtons[i].setPosition({ 50 + col * 240.0f, baseY });

        sf::FloatRect textBounds = interiorStyleLabels[i].getLocalBounds();
        interiorStyleLabels[i].setPosition({ 50 + col * 240.0f + (220 - textBounds.size.x) / 2,
                                          baseY + (40 - textBounds.size.y) / 2 - 3 });
    }

    // Update both group labels to scroll with content
    artisticGroupLabel.setPosition({ 50, 240 + artisticScrollOffset });
    interiorGroupLabel.setPosition({ 50, interiorStartY - 40 + artisticScrollOffset }); // Change -30 to -50
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

    // Draw model buttons
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
        if (artisticLabelPos.y >= 240 && artisticLabelPos.y <= 620) { // Strict bounds
            window.draw(artisticGroupLabel);
        }

        // Draw artistic style buttons (with strict bounds checking)
        for (size_t i = 0; i < artisticStyleButtons.size(); i++) {
            sf::Vector2f pos = artisticStyleButtons[i].getPosition();
            if (pos.y >= 240 && pos.y <= 620) { // Only draw within scroll area
                window.draw(artisticStyleButtons[i]);
                window.draw(artisticStyleLabels[i]);
            }
        }

        // Draw interior group label if visible
        sf::Vector2f labelPos = interiorGroupLabel.getPosition();
        if (labelPos.y >= 240 && labelPos.y <= 620) {
            window.draw(interiorGroupLabel);
        }

        // Draw interior design style buttons (with strict bounds checking)
        for (size_t i = 0; i < interiorStyleButtons.size(); i++) {
            sf::Vector2f pos = interiorStyleButtons[i].getPosition();
            if (pos.y >= 240 && pos.y <= 620) { // Only draw within scroll area
                window.draw(interiorStyleButtons[i]);
                window.draw(interiorStyleLabels[i]);
            }
        }
    }
    else {
        // Draw traditional styles header and buttons (Studio Ghibli, Photorealistic)
        window.draw(stylesGroupLabel);
        window.draw(ghibliButton);
        window.draw(ghibliLabel);
        window.draw(photorealisticButton);
        window.draw(photorealisticLabel);
    }

    window.draw(generateButton);
    window.draw(generateLabel);
}

void ImageGenerator::renderLoadingScreen() {
    window.draw(loadingText);
}

void ImageGenerator::renderImageDisplay() {
    window.draw(imageSprite);
    window.draw(newImageButton);
    window.draw(newImageLabel);
}
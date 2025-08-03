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

    // Model selection buttons (Realism vs Aesthetic)
    for (int i = 0; i < 2; i++) {
        sf::RectangleShape button({ 200, 50 });
        button.setPosition({ 312 + i * 220.0f, 140 });
        button.setFillColor(i == 0 ? selectedButtonColor : buttonColor);
        modelButtons.push_back(button);

        sf::Text label(font, "");
        label.setFont(font);
        label.setString(modelNames[i]);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 312 + i * 220.0f + (200 - textBounds.size.x) / 2,
                        140 + (50 - textBounds.size.y) / 2 - 5 });
        modelLabels.push_back(label);
    }

    // Style buttons - Studio Ghibli and Photorealistic
    ghibliButton.setSize({ 200, 60 });
    ghibliButton.setPosition({ 312, 200 });
    ghibliButton.setFillColor(buttonColor);

    ghibliLabel.setFont(font);
    ghibliLabel.setString("Studio Ghibli");
    ghibliLabel.setCharacterSize(18);
    ghibliLabel.setFillColor(sf::Color::White);
    sf::FloatRect ghibliBounds = ghibliLabel.getLocalBounds();
    ghibliLabel.setPosition({ 312 + (200 - ghibliBounds.size.x) / 2,
                    200 + (60 - ghibliBounds.size.y) / 2 - 5 });

    photorealisticButton.setSize({ 200, 60 });
    photorealisticButton.setPosition({ 532, 200 });
    photorealisticButton.setFillColor(buttonColor);

    photorealisticLabel.setFont(font);
    photorealisticLabel.setString("Photorealistic");
    photorealisticLabel.setCharacterSize(18);
    photorealisticLabel.setFillColor(sf::Color::White);
    sf::FloatRect photoBounds = photorealisticLabel.getLocalBounds();
    photorealisticLabel.setPosition({ 532 + (200 - photoBounds.size.x) / 2,
                    200 + (60 - photoBounds.size.y) / 2 - 5 });

    // Generate button
    generateButton.setSize({ 200, 60 });
    generateButton.setPosition({ 412, 280 });
    generateButton.setFillColor(sf::Color(50, 150, 50));

    generateLabel.setFont(font);
    generateLabel.setString("Generate Image");
    generateLabel.setCharacterSize(20);
    generateLabel.setFillColor(sf::Color::White);
    sf::FloatRect genBounds = generateLabel.getLocalBounds();
    generateLabel.setPosition({ 412 + (200 - genBounds.size.x) / 2, 295 });

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

void ImageGenerator::updateButtonHovers(sf::Vector2i mousePos) {
    // Generate button hover
    if (generateButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        generateButton.setFillColor(sf::Color(70, 170, 70));
    }
    else {
        generateButton.setFillColor(sf::Color(50, 150, 50));
    }

    // Studio Ghibli button hover
    if (ghibliButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
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
    if (photorealisticButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        if (selectedStyle != StyleMode::PHOTOREALISTIC) {
            photorealisticButton.setFillColor(buttonHoverColor);
        }
    }
    else {
        if (selectedStyle != StyleMode::PHOTOREALISTIC) {
            photorealisticButton.setFillColor(buttonColor);
        }
    }

    // Model button hovers
    for (int i = 0; i < modelButtons.size(); i++) {
        if (modelButtons[i].getGlobalBounds().contains(sf::Vector2f(mousePos))) {
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

    for (const auto& button : modelButtons) {
        window.draw(button);
    }
    for (const auto& label : modelLabels) {
        window.draw(label);
    }

    window.draw(ghibliButton);
    window.draw(ghibliLabel);
    window.draw(photorealisticButton);
    window.draw(photorealisticLabel);

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
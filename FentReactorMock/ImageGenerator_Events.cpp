#include "ImageGenerator.h"

void ImageGenerator::handleEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (currentState == AppState::INPUT_SCREEN) {
            handleInputScreenEvents(*event);
        }
        else if (currentState == AppState::IMAGE_DISPLAY) {
            handleImageDisplayEvents(*event);
        }
    }
}

void ImageGenerator::handleInputScreenEvents(sf::Event& event) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        // Check prompt box click
        if (promptBox.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            promptActive = true;
            promptBox.setOutlineColor(sf::Color::Blue);

            // Calculate cursor position based on mouse click
            float clickX = mousePos.x - promptBox.getPosition().x - 10; // Account for padding
            sf::Text tempText(font, "");
            tempText.setCharacterSize(20);

            cursorPosition = 0;
            for (size_t i = 0; i <= userPrompt.length(); i++) {
                tempText.setString(userPrompt.substr(0, i));
                float textWidth = tempText.getLocalBounds().size.x;

                if (textWidth > clickX) {
                    // Check which side of the character is closer
                    if (i > 0) {
                        tempText.setString(userPrompt.substr(0, i - 1));
                        float prevWidth = tempText.getLocalBounds().size.x;
                        if (clickX - prevWidth < textWidth - clickX) {
                            cursorPosition = i - 1;
                        }
                        else {
                            cursorPosition = i;
                        }
                    }
                    else {
                        cursorPosition = 0;
                    }
                    break;
                }
                cursorPosition = i;
            }
            updateCursorPosition();
        }
        else {
            promptActive = false;
            promptBox.setOutlineColor(sf::Color(100, 100, 100));
        }

        // Check model button clicks
        for (int i = 0; i < modelButtons.size(); i++) {
            if (modelButtons[i].getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                // Don't allow switching to Aesthetic if Photorealistic is selected
                if (i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC) {
                    continue; // Block the click
                }

                selectedModel = static_cast<APIModel>(i);
                updateModelButtons();
            }
        }

        // Check Studio Ghibli button click
        if (ghibliButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            selectedStyle = (selectedStyle == StyleMode::STUDIO_GHIBLI) ? StyleMode::NONE : StyleMode::STUDIO_GHIBLI;
            updateStyleButtons();
        }

        // Check Photorealistic button click
        if (photorealisticButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            selectedStyle = (selectedStyle == StyleMode::PHOTOREALISTIC) ? StyleMode::NONE : StyleMode::PHOTOREALISTIC;

            // Auto-switch to Realism if Photorealistic is selected
            if (selectedStyle == StyleMode::PHOTOREALISTIC) {
                selectedModel = APIModel::REALISM;
                updateModelButtons();
                std::cout << "Auto-switched to Realism API for photorealistic style" << std::endl;
            }

            updateStyleButtons();
        }

        // Check generate button
        if (generateButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            if (!userPrompt.empty()) {
                generateImage();
            }
        }
    }

    // Handle text input - UPDATED VERSION
    if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
        if (promptActive) {
            if (textEntered->unicode == 8) { // Backspace
                if (!userPrompt.empty() && cursorPosition > 0) {
                    userPrompt.erase(cursorPosition - 1, 1);
                    cursorPosition--;
                    promptText.setString(userPrompt);
                    updateCursorPosition();
                }
            }
            else if (textEntered->unicode >= 32 && textEntered->unicode < 127) {
                if (userPrompt.length() < 2000) { // Increased limit from 100 to 2000
                    userPrompt.insert(cursorPosition, 1, static_cast<char>(textEntered->unicode));
                    cursorPosition++;
                    promptText.setString(userPrompt);
                    updateCursorPosition();
                }
            }
        }
    }

    // Handle arrow key navigation - CORRECT SFML ENUM NAMES
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (promptActive) {
            if (keyPressed->code == sf::Keyboard::Key::Left && cursorPosition > 0) {
                cursorPosition--;
                updateCursorPosition();
            }
            else if (keyPressed->code == sf::Keyboard::Key::Right && cursorPosition < userPrompt.length()) {
                cursorPosition++;
                updateCursorPosition();
            }
            else if (keyPressed->code == sf::Keyboard::Key::Home) {
                cursorPosition = 0;
                updateCursorPosition();
            }
            else if (keyPressed->code == sf::Keyboard::Key::End) {
                cursorPosition = userPrompt.length();
                updateCursorPosition();
            }
        }
    }

    // Update button hover effects
    updateButtonHovers(mousePos);
}

void ImageGenerator::handleImageDisplayEvents(sf::Event& event) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        if (newImageButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            currentState = AppState::INPUT_SCREEN;
        }
    }
}
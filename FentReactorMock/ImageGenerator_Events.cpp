#include "ImageGenerator.h"

void ImageGenerator::handleEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        // Handle window resize to maintain aspect ratio
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            handleWindowResize();
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::F11) {
                static bool isFullscreen = false;

                if (isFullscreen) {
                    // Use desktop mode and style, then resize to 1024x768 manually
                    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
                    window.create(desktopMode, "AI Image Generator", sf::Style::Default);
                    window.setSize({ 1024, 768 }); // Resize to desired windowed size
                    isFullscreen = false;
                }
                else {
                    // Fullscreen mode using best fullscreen resolution
                    sf::VideoMode fullscreenMode = sf::VideoMode::getFullscreenModes()[0];
                    window.create(fullscreenMode, "AI Image Generator", sf::Style::None);
                    isFullscreen = true;
                }

                setupView();
                handleWindowResize();
            }
        }

        if (currentState == AppState::INPUT_SCREEN) {
            handleInputScreenEvents(event.value());
            handleScroll(event.value());
        }
        else if (currentState == AppState::IMAGE_DISPLAY) {
            handleImageDisplayEvents(event.value());
        }
        else if (currentState == AppState::GALLERY_SCREEN) {
            handleGalleryScreenEvents(event.value());
        }
    }
}

void ImageGenerator::handleScroll(sf::Event& event) {
    // Handle scrolling for categories that need it
    if (selectedModel == APIModel::ARTISTIC ||
        selectedModel == APIModel::GAMING_TECH ||
        selectedModel == APIModel::ENTERTAINMENT ||
        selectedModel == APIModel::SPECIALTY_ROOMS) {

        if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
            // Transform mouse position to logical coordinates
            sf::Vector2i screenMousePos = sf::Mouse::getPosition(window);
            sf::Vector2f logicalMousePos = getLogicalMousePosition(screenMousePos);

            // Check if mouse is in scroll area using logical coordinates
            if (artisticScrollArea.getGlobalBounds().contains(logicalMousePos)) {
                float scrollSpeed = 30.0f;
                artisticScrollOffset += mouseWheel->delta * scrollSpeed;

                // Calculate total content height based on current category
                float totalContentHeight = 0;
                if (selectedModel == APIModel::ARTISTIC) {
                    int artisticRows = (artisticStyles.size() + 3) / 4;
                    int interiorRows = (interiorStyles.size() + 3) / 4;
                    totalContentHeight = 30 + (artisticRows * 50) + 50 + (interiorRows * 50);
                }
                else {
                    // For other categories, calculate based on current category styles
                    int rows = (currentCategoryStyles.size() + 3) / 4;
                    totalContentHeight = 30 + (rows * 50);
                }

                float viewHeight = 380; // Fixed scroll area height

                // STRICT scroll bounds - content must stay within designated area
                float maxScroll = 0;  // Never scroll above starting position
                float minScroll = std::min(0.0f, viewHeight - totalContentHeight);

                artisticScrollOffset = std::max(minScroll, std::min(maxScroll, artisticScrollOffset));

                if (selectedModel == APIModel::ARTISTIC) {
                    updateArtisticButtonPositions();
                }
                else {
                    updateCategoryStylePositions();
                }
            }
        }
    }
}

void ImageGenerator::handleInputScreenEvents(sf::Event& event) {
    // Transform mouse position to logical coordinates
    sf::Vector2i screenMousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePos = getLogicalMousePosition(screenMousePos);

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        // Check prompt box click
        if (promptBox.getGlobalBounds().contains(mousePos)) {
            promptActive = true;
            promptBox.setOutlineColor(sf::Color::Blue);

            // Calculate cursor position based on mouse click
            float clickX = mousePos.x - promptBox.getPosition().x - 10; // Account for padding
            sf::Text tempText(font, "", 20);

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

        // Check model button clicks (now 8 categories)
        for (int i = 0; i < modelButtons.size(); i++) {
            if (modelButtons[i].getGlobalBounds().contains(mousePos)) {
                // Don't allow switching to Aesthetic if Photorealistic is selected
                if (i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC) {
                    continue; // Block the click
                }

                selectedModel = static_cast<APIModel>(i);

                // Reset style selection when switching categories
                selectedStyle = StyleMode::NONE;
                artisticScrollOffset = 0; // Reset scroll when switching categories

                updateModelButtons();
                updateStyleButtons();

                // Update button positions for new category
                if (selectedModel == APIModel::ARTISTIC) {
                    updateArtisticButtonPositions();
                }
                else {
                    updateCategoryStylePositions();
                }
            }
        }

        // Handle style button clicks based on current model
        if (selectedModel == APIModel::ARTISTIC) {
            // Check artistic style button clicks
            for (size_t i = 0; i < artisticStyleButtons.size(); i++) {
                if (artisticStyleButtons[i].getGlobalBounds().contains(mousePos)) {
                    selectedStyle = (selectedStyle == artisticStyles[i]) ? StyleMode::NONE : artisticStyles[i];
                    updateStyleButtons();
                    break;
                }
            }

            // Check interior style button clicks
            for (size_t i = 0; i < interiorStyleButtons.size(); i++) {
                if (interiorStyleButtons[i].getGlobalBounds().contains(mousePos)) {
                    selectedStyle = (selectedStyle == interiorStyles[i]) ? StyleMode::NONE : interiorStyles[i];
                    updateStyleButtons();
                    break;
                }
            }
        }
        else if (selectedModel == APIModel::REALISM || selectedModel == APIModel::AESTHETIC) {
            // Legacy style buttons for Realism/Aesthetic
            // Check Studio Ghibli button click
            if (ghibliButton.getGlobalBounds().contains(mousePos)) {
                selectedStyle = (selectedStyle == StyleMode::STUDIO_GHIBLI) ? StyleMode::NONE : StyleMode::STUDIO_GHIBLI;
                updateStyleButtons();
            }

            // Check Photorealistic button click
            if (photorealisticButton.getGlobalBounds().contains(mousePos)) {
                selectedStyle = (selectedStyle == StyleMode::PHOTOREALISTIC) ? StyleMode::NONE : StyleMode::PHOTOREALISTIC;

                // Auto-switch to Realism if Photorealistic is selected
                if (selectedStyle == StyleMode::PHOTOREALISTIC) {
                    selectedModel = APIModel::REALISM;
                    updateModelButtons();
                    std::cout << "Auto-switched to Realism API for photorealistic style" << std::endl;
                }

                updateStyleButtons();
            }
        }
        else {
            // Handle new category style button clicks
            for (size_t i = 0; i < categoryStyleButtons.size(); i++) {
                if (categoryStyleButtons[i].getGlobalBounds().contains(mousePos)) {
                    selectedStyle = (selectedStyle == currentCategoryStyles[i]) ? StyleMode::NONE : currentCategoryStyles[i];
                    updateStyleButtons();
                    break;
                }
            }
        }

        // Check generate button
        if (generateButton.getGlobalBounds().contains(mousePos)) {
            if (!userPrompt.empty()) {
                generateImage();
            }
        }

        // Check orientation toggle button
        if (orientationButton.getGlobalBounds().contains(mousePos)) {
            globalOrientation = (globalOrientation == OrientationMode::PORTRAIT) ?
                OrientationMode::LANDSCAPE : OrientationMode::PORTRAIT;

            orientationLabel.setString(globalOrientation == OrientationMode::PORTRAIT ? "Portrait" : "Landscape");
            sf::FloatRect orientBounds = orientationLabel.getLocalBounds();
            orientationLabel.setPosition({ 532 + (120 - orientBounds.size.x) / 2, 715 });

            std::cout << "Orientation changed to: " <<
                (globalOrientation == OrientationMode::PORTRAIT ? "Portrait" : "Landscape") << std::endl;
        }

        // Check gallery button
        if (galleryButton.getGlobalBounds().contains(mousePos)) {
            currentState = AppState::GALLERY_SCREEN;
            updateGalleryDisplay();
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

    // Update button hover effects using logical coordinates
    updateButtonHovers(mousePos);
}

void ImageGenerator::handleImageDisplayEvents(sf::Event& event) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        // Transform mouse position to logical coordinates
        sf::Vector2i screenMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = getLogicalMousePosition(screenMousePos);

        if (newImageButton.getGlobalBounds().contains(mousePos)) {
            currentState = AppState::INPUT_SCREEN;
        }

        if (saveImageButton.getGlobalBounds().contains(mousePos)) {
            saveCurrentImage();
        }
    }
}

void ImageGenerator::handleGalleryScreenEvents(sf::Event& event) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2i screenMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = getLogicalMousePosition(screenMousePos);

        // Back to main button
        if (backToMainButton.getGlobalBounds().contains(mousePos)) {
            currentState = AppState::INPUT_SCREEN;
        }

        // Portrait tab button
        if (portraitTabButton.getGlobalBounds().contains(mousePos)) {
            showingPortraitGallery = true;
            galleryScrollOffset = 0;
            portraitTabButton.setFillColor(selectedButtonColor);
            landscapeTabButton.setFillColor(buttonColor);
            updateGalleryDisplay();
        }

        // Landscape tab button
        if (landscapeTabButton.getGlobalBounds().contains(mousePos)) {
            showingPortraitGallery = false;
            galleryScrollOffset = 0;
            portraitTabButton.setFillColor(buttonColor);
            landscapeTabButton.setFillColor(selectedButtonColor);
            updateGalleryDisplay();
        }
    }

    // Handle gallery scrolling
    if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        sf::Vector2i screenMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f logicalMousePos = getLogicalMousePosition(screenMousePos);

        if (galleryScrollArea.getGlobalBounds().contains(logicalMousePos)) {
            auto currentImages = getCurrentGalleryImages();
            int imagesPerRow = 4;
            int rows = (currentImages.size() + imagesPerRow - 1) / imagesPerRow;
            int maxScroll = std::max(0, (rows * 120) - static_cast<int>(galleryScrollArea.getSize().y));

            galleryScrollOffset -= static_cast<int>(mouseWheel->delta * 30);
            galleryScrollOffset = std::max(0, std::min(maxScroll, galleryScrollOffset));
        }
    }
}
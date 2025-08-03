#include "ImageGenerator.h"
#include <chrono>
#include <fstream>

using json = nlohmann::json;

void ImageGenerator::generateImage() {
    currentState = AppState::LOADING;

    std::thread([this]() {
        std::cout << "Starting API request..." << std::endl;

        // Make API request using selected model
        std::string requestId = makeAPIRequest(userPrompt, getStylePromptModifier(selectedStyle), selectedModel);

        if (requestId.empty()) {
            std::cout << "Failed to submit API request" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        std::cout << "Request submitted with ID: " << requestId << std::endl;

        // Poll for completion
        std::string imageUrl;
        int maxAttempts = 60; // 60 attempts * 2 seconds = 2 minutes max

        for (int attempt = 0; attempt < maxAttempts; attempt++) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::string statusResponse = pollRequestStatus(requestId, selectedModel);
            if (statusResponse.empty()) {
                std::cout << "Failed to get status" << std::endl;
                continue;
            }

            try {
                json statusJson = json::parse(statusResponse);

                // Check if the response contains images (meaning it's completed)
                if (statusJson.contains("images") && statusJson["images"].is_array() && !statusJson["images"].empty()) {
                    imageUrl = statusJson["images"][0]["url"];
                    std::cout << "Image generation completed!" << std::endl;
                    break;
                }

                std::string status = "UNKNOWN";

                // Handle status field if it exists
                if (statusJson.contains("status") && statusJson["status"].is_string()) {
                    status = statusJson["status"];
                }
                else if (statusJson.contains("status") && statusJson["status"].is_null()) {
                    status = "QUEUED";
                }
                else {
                    // If no status field but no images either, still processing
                    status = "PROCESSING";
                }

                std::cout << "Status: " << status << " (attempt " << (attempt + 1) << "/" << maxAttempts << ")" << std::endl;

                if (status == "FAILED") {
                    std::cout << "API request failed" << std::endl;
                    currentState = AppState::INPUT_SCREEN;
                    return;
                }
                // Continue polling for any other status
            }
            catch (const std::exception& e) {
                std::cout << "Error parsing status: " << e.what() << std::endl;
                std::cout << "Raw response: '" << statusResponse << "'" << std::endl;

                // If we can't parse the response, it might be an HTTP error
                if (statusResponse.find("404") != std::string::npos ||
                    statusResponse.find("Not Found") != std::string::npos) {
                    std::cout << "Endpoint not found - check API URL" << std::endl;
                    currentState = AppState::INPUT_SCREEN;
                    return;
                }
            }
        }

        if (imageUrl.empty()) {
            std::cout << "Timeout waiting for image generation" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        std::cout << "Image URL received: " << imageUrl << std::endl;

        // Download image
        std::string filename = "generated_image.jpg";
        if (!downloadImage(imageUrl, filename)) {
            std::cout << "Failed to download image" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        // Load image into SFML
        if (imageTexture.loadFromFile(filename)) {
            std::cout << "High-resolution image loaded successfully" << std::endl;
            std::cout << "Image resolution: " << imageTexture.getSize().x << "x" << imageTexture.getSize().y << std::endl;
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

            std::cout << "Image positioned and scaled" << std::endl;
        }
        else {
            std::cout << "Failed to load image file" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        currentState = AppState::IMAGE_DISPLAY;
        }).detach();
}

// HTTP callback function
size_t ImageGenerator::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

std::string ImageGenerator::makeAPIRequest(const std::string& prompt, const std::string& styleModifier, APIModel model) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    // Get API key from environment variable
    const char* apiKey = std::getenv("FAL_KEY");
    if (!apiKey) {
        std::cout << "ERROR: FAL_KEY environment variable not set!" << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }

    // Prepare headers
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Key " + std::string(apiKey);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader.c_str());

    // Prepare JSON payload based on selected model - HIGH RESOLUTION
    json payload;
    std::string apiUrl;

    if (model == APIModel::REALISM) {
        // FLUX schnell API - Custom high resolution
        apiUrl = "https://queue.fal.run/fal-ai/flux-1/schnell";
        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", 1296}, {"height", 2304}}}, // High res 9:16 ratio
            {"num_inference_steps", 4},
            {"guidance_scale", 3.5},
            {"num_images", 1},
            {"enable_safety_checker", true},
            {"output_format", "jpeg"}
        };
    }
    else if (model == APIModel::AESTHETIC) {
        // Playground v2.5 API - High resolution 9:16 aspect ratio  
        apiUrl = "https://queue.fal.run/fal-ai/playground-v25";
        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", 1296}, {"height", 2304}}}, // High res 9:16 ratio
            {"num_images", 1},
            {"guidance_scale", 7},
            {"format", "jpeg"},
            {"num_inference_steps", 50}
        };
    }
    else { // APIModel::ARTISTIC
        // FLUX LoRA API - Best balance of speed and artistic quality
        apiUrl = "https://queue.fal.run/fal-ai/flux-lora";
        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", 1296}, {"height", 2304}}}, // High res 9:16 ratio
            {"num_inference_steps", 28}, // Good balance of speed and quality
            {"guidance_scale", 3.5},
            {"num_images", 1},
            {"enable_safety_checker", true},
            {"output_format", "jpeg"}
        };
    }

    std::string jsonString = payload.dump();

    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Make request
    res = curl_easy_perform(curl);

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("request_id")) {
            return responseJson["request_id"];
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error parsing API response: " << e.what() << std::endl;
        std::cout << "Response: " << response << std::endl;
    }

    return "";
}

std::string ImageGenerator::pollRequestStatus(const std::string& requestId, APIModel model) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    const char* apiKey = std::getenv("FAL_KEY");
    if (!apiKey) {
        curl_easy_cleanup(curl);
        return "";
    }

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Key " + std::string(apiKey);
    headers = curl_slist_append(headers, authHeader.c_str());

    // Build URL based on model
    std::string url;
    if (model == APIModel::REALISM) {
        url = "https://queue.fal.run/fal-ai/flux-1/requests/" + requestId;
    }
    else if (model == APIModel::AESTHETIC) {
        url = "https://queue.fal.run/fal-ai/playground-v25/requests/" + requestId;
    }
    else { // APIModel::ARTISTIC
        url = "https://queue.fal.run/fal-ai/flux-lora/requests/" + requestId;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response;
}

bool ImageGenerator::downloadImage(const std::string& imageUrl, const std::string& filename) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, imageUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}

std::string ImageGenerator::getStylePromptModifier(StyleMode style) {
    switch (style) {
    case StyleMode::STUDIO_GHIBLI:
        return ", in the style of Studio Ghibli";
    case StyleMode::PHOTOREALISTIC:
        return ", photorealistic photography, ultra realistic, highly detailed, 8k resolution, professional photography, sharp focus, real world";

        // Artistic styles
    case StyleMode::IMPRESSIONISM:
        return ", impressionist painting style, loose brushwork, light and color emphasis, plein air technique";
    case StyleMode::ABSTRACT_EXPRESSIONISM:
        return ", abstract expressionist painting, bold colors, emotional intensity, non-representational";
    case StyleMode::CUBISM:
        return ", cubist painting style, geometric forms, multiple perspectives, fragmented composition";
    case StyleMode::ART_DECO:
        return ", Art Deco style painting, geometric patterns, bold lines, luxury aesthetic, 1920s design";
    case StyleMode::POP_ART:
        return ", pop art style, bright colors, commercial imagery, bold graphics, contemporary culture";
    case StyleMode::REALISM_ART:
        return ", photorealistic oil painting style, hyperrealistic digital art, detailed brush techniques, fine art realism";
    case StyleMode::EXPRESSIONISM:
        return ", expressionist painting, emotional intensity, distorted forms, vivid colors";
    case StyleMode::BAROQUE:
        return ", baroque painting style, dramatic lighting, rich colors, ornate details, classical composition";
    case StyleMode::FAUVISM:
        return ", fauvist painting style, wild colors, bold brushstrokes, expressive use of color";
    case StyleMode::NEOCLASSICISM:
        return ", neoclassical painting style, classical subjects, balanced composition, idealized forms";
    case StyleMode::FUTURISM:
        return ", futurist painting style, dynamic movement, mechanical forms, speed and technology";
    case StyleMode::SURREALISM:
        return ", surrealist painting, dreamlike imagery, unexpected juxtapositions, subconscious exploration";
    case StyleMode::RENAISSANCE:
        return ", Renaissance painting style, classical techniques, realistic proportions, religious or mythological themes";
    case StyleMode::ACADEMIC_ART:
        return ", academic art style, classical training, realistic representation, traditional techniques";
    case StyleMode::ANALYTICAL_ART:
        return ", analytical art approach, systematic study, geometric analysis, structural composition";
    case StyleMode::BAUHAUS:
        return ", Bauhaus art style, functional design, geometric forms, modernist aesthetic";
    case StyleMode::CONCEPTUAL_ART:
        return ", conceptual art style, idea-based artwork, minimal execution, thought-provoking";
    case StyleMode::CONSTRUCTIVISM:
        return ", constructivist art style, abstract geometric forms, revolutionary aesthetics, industrial materials";
    case StyleMode::DADA:
        return ", dadaist art style, anti-art movement, absurd imagery, collage techniques";
    case StyleMode::GEOMETRIC_ABSTRACTION:
        return ", geometric abstract painting, mathematical precision, clean lines, color relationships";
    case StyleMode::MINIMALISM_ART:
        return ", minimalist art style, simple forms, reduced elements, essential composition";
    case StyleMode::NEO_IMPRESSIONISM:
        return ", neo-impressionist painting, pointillist technique, scientific color theory, optical mixing";
    case StyleMode::POST_IMPRESSIONISM:
        return ", post-impressionist painting style, symbolic content, synthetic color, expressive brushwork";

        // Interior design styles - more specific to avoid room generation
    case StyleMode::MID_CENTURY_MODERN:
        return ", abstract art piece with mid-century modern aesthetic, clean geometric shapes, retro color palette, minimalist composition, wall art";
    case StyleMode::BOHEMIAN:
        return ", bohemian art piece, eclectic artistic composition, vibrant earth tones, free-spirited design, textile-inspired patterns, wall art";
    case StyleMode::MINIMALISM_DESIGN:
        return ", minimalist art piece, simple geometric forms, neutral color scheme, clean composition, negative space, wall art";
    case StyleMode::SCANDINAVIAN:
        return ", Scandinavian-inspired art piece, light natural colors, simple organic shapes, hygge aesthetic, wall art";
    case StyleMode::ART_DECO_DESIGN:
        return ", Art Deco art piece, geometric patterns, gold and black color scheme, luxury design elements, wall art";
    case StyleMode::FARMHOUSE:
        return ", farmhouse-style art piece, rustic textures, natural wood tones, vintage americana, cozy aesthetic, wall art";
    case StyleMode::INDUSTRIAL:
        return ", industrial-style art piece, metallic textures, exposed materials aesthetic, urban design, wall art";
    case StyleMode::CONTEMPORARY:
        return ", contemporary art piece, modern design elements, sophisticated color palette, current artistic trends, wall art";
    case StyleMode::TRADITIONAL:
        return ", traditional art piece, classic design motifs, refined color scheme, timeless composition, wall art";
    case StyleMode::RUSTIC:
        return ", rustic art piece, natural weathered textures, earth tone palette, countryside aesthetic, wall art";
    case StyleMode::TRANSITIONAL:
        return ", transitional art piece, balanced modern and classic elements, neutral sophisticated palette, wall art";
    case StyleMode::FRENCH_COUNTRY:
        return ", French country art piece, pastoral motifs, soft romantic colors, vintage charm, wall art";
    case StyleMode::JAPANDI:
        return ", Japandi art piece, zen minimalism, natural wood and stone textures, peaceful composition, wall art";
    case StyleMode::MEDITERRANEAN:
        return ", Mediterranean art piece, warm terracotta and blue colors, coastal-inspired design, relaxed elegance, wall art";
    case StyleMode::SHABBY_CHIC:
        return ", shabby chic art piece, vintage distressed textures, soft pastel colors, romantic feminine aesthetic, wall art";
    case StyleMode::ECLECTIC:
        return ", eclectic art piece, mixed artistic styles, bold creative composition, diverse design elements, wall art";
    case StyleMode::REGENCY:
        return ", Regency-style art piece, elegant classical motifs, sophisticated color palette, refined composition, wall art";
    case StyleMode::COASTAL:
        return ", coastal art piece, ocean-inspired colors, beach aesthetic, nautical design elements, wall art";
    case StyleMode::MAXIMALISM:
        return ", maximalist art piece, bold patterns and colors, rich decorative elements, abundant visual interest, wall art";

    case StyleMode::NONE:
    default:
        return "";
    }
}
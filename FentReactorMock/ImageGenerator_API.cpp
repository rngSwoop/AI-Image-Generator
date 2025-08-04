#include "ImageGenerator.h"
#include <chrono>
#include <fstream>

using json = nlohmann::json;

APIModel ImageGenerator::getAPIForModel(APIModel selectedModel) {
    // Map the new category models to actual APIs
    switch (selectedModel) {
    case APIModel::REALISM:
        return APIModel::REALISM;        // FLUX schnell
    case APIModel::AESTHETIC:
        return APIModel::AESTHETIC;      // Playground v2.5
    case APIModel::ARTISTIC:
        return APIModel::ARTISTIC;       // FLUX LoRA
    case APIModel::GAMING_TECH:
        return APIModel::ARTISTIC;       // FLUX LoRA (best for stylized content)
    case APIModel::ENTERTAINMENT:
        return APIModel::ARTISTIC;       // FLUX LoRA (good at dramatic compositions)
    case APIModel::PROFESSIONAL:
        return APIModel::REALISM;        // FLUX schnell (fast, clean, corporate)
    case APIModel::SPECIALTY_ROOMS:
        return APIModel::ARTISTIC;       // FLUX LoRA (handles themed content well)
    case APIModel::LANDSCAPES:
        return APIModel::REALISM;        // FLUX schnell (fast photorealistic nature)
    default:
        return APIModel::REALISM;
    }
}

void ImageGenerator::generateImage() {
    currentState = AppState::LOADING;

    std::thread([this]() {
        std::cout << "Starting API request..." << std::endl;

        // Get the actual API to use for the selected category
        APIModel actualAPI = getAPIForModel(selectedModel);

        // Make API request using mapped API
        std::string requestId = makeAPIRequest(userPrompt, getStylePromptModifier(selectedStyle), actualAPI);

        if (requestId.empty()) {
            std::cout << "Failed to submit API request" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        std::cout << "Request submitted with ID: " << requestId << std::endl;

        // Poll for completion using the actual API
        std::string imageUrl;
        int maxAttempts = 60; // 60 attempts * 2 seconds = 2 minutes max

        for (int attempt = 0; attempt < maxAttempts; attempt++) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::string statusResponse = pollRequestStatus(requestId, actualAPI);
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

        // CRITICAL: Set the current generated image path IMMEDIATELY after successful download
        currentGeneratedImagePath = filename;
        std::cout << "Set currentGeneratedImagePath to: " << currentGeneratedImagePath << std::endl;

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

            // Update button positions based on image orientation
            updateImageDisplayButtonPositions();

            hasGeneratedImage = true;
        }
        else {
            std::cout << "Failed to load image file" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        // CRITICAL: Invalidate cache AFTER everything is set up properly
        invalidateAlreadySavedCache();

        currentState = AppState::IMAGE_DISPLAY;
        viewingFromGallery = false; // CRITICAL: Ensure we're not in gallery viewing mode after fresh generation
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
        // FLUX schnell API - Dynamic resolution based on orientation
        apiUrl = "https://queue.fal.run/fal-ai/flux-1/schnell";

        int width, height;
        if (globalOrientation == OrientationMode::PORTRAIT) {
            width = 1296; height = 2304; // 9:16
        }
        else {
            width = 2304; height = 1296; // 16:9
        }

        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", width}, {"height", height}}},
            {"num_inference_steps", 4},
            {"guidance_scale", 3.5},
            {"num_images", 1},
            {"enable_safety_checker", true},
            {"output_format", "jpeg"}
        };
    }
    else if (model == APIModel::AESTHETIC) {
        // Playground v2.5 API - Dynamic resolution based on orientation
        apiUrl = "https://queue.fal.run/fal-ai/playground-v25";

        int width, height;
        if (globalOrientation == OrientationMode::PORTRAIT) {
            width = 1296; height = 2304; // 9:16
        }
        else {
            width = 2304; height = 1296; // 16:9
        }

        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", width}, {"height", height}}},
            {"num_images", 1},
            {"guidance_scale", 7},
            {"format", "jpeg"},
            {"num_inference_steps", 50}
        };
    }
    else { // APIModel::ARTISTIC (and all other categories that map to it)
        // FLUX LoRA API - Dynamic resolution based on orientation
        apiUrl = "https://queue.fal.run/fal-ai/flux-lora";

        int width, height;
        if (globalOrientation == OrientationMode::PORTRAIT) {
            width = 1296; height = 2304; // 9:16
        }
        else {
            width = 2304; height = 1296; // 16:9
        }

        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", width}, {"height", height}}},
            {"num_inference_steps", 28},
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
    else { // APIModel::ARTISTIC (and all categories that map to it)
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

        // Artistic styles (existing)
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

        // Interior design styles (existing)
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

        // Gaming & Tech styles (NEW)
    case StyleMode::CYBERPUNK:
        return ", cyberpunk art style, neon lights, dark urban atmosphere, futuristic technology, digital rain effects, purple and cyan color scheme, high-tech aesthetic";
    case StyleMode::SYNTHWAVE:
        return ", synthwave aesthetic, retro-futuristic, neon pink and blue gradients, geometric shapes, 1980s sci-fi atmosphere, grid patterns, outrun style";
    case StyleMode::PIXEL_ART:
        return ", pixel art style, 8-bit graphics, retro gaming aesthetic, blocky forms, limited color palette, digital art, pixelated composition";
    case StyleMode::ANIME_MANGA:
        return ", anime manga art style, cel-shaded illustration, vibrant colors, Japanese animation aesthetic, clean line art, expressive character design";
    case StyleMode::SCI_FI_TECH:
        return ", sci-fi technology art, futuristic interface design, holographic elements, circuit board patterns, advanced technology aesthetic, digital interfaces";
    case StyleMode::RETRO_GAMING:
        return ", retro gaming art style, arcade aesthetics, vintage video game graphics, nostalgic gaming atmosphere, classic console era design";

        // Entertainment styles (NEW)
    case StyleMode::MOVIE_POSTER:
        return ", movie poster composition, dramatic lighting, cinematic framing, bold visual hierarchy, theatrical atmosphere, film marketing aesthetic, dynamic layout";
    case StyleMode::FILM_NOIR:
        return ", film noir style, high contrast black and white, dramatic shadows, venetian blind lighting, 1940s atmosphere, mysterious mood, chiaroscuro lighting";
    case StyleMode::CONCERT_POSTER:
        return ", concert poster design, music venue aesthetics, rock poster style, bold typography space, energetic composition, live music atmosphere";
    case StyleMode::SPORTS_MEMORABILIA:
        return ", sports memorabilia style, team colors, athletic imagery, stadium atmosphere, championship aesthetic, competitive sports design";
    case StyleMode::VINTAGE_CINEMA:
        return ", vintage cinema poster, classic Hollywood glamour, golden age of film, retro movie advertising, nostalgic film aesthetic";

        // Professional styles (NEW)
    case StyleMode::CORPORATE_MODERN:
        return ", corporate modern art, clean professional aesthetic, sophisticated color palette, minimalist business environment, executive office style, contemporary corporate design";
    case StyleMode::ABSTRACT_CORPORATE:
        return ", abstract corporate art, professional artistic composition, sophisticated geometric forms, business-appropriate design, modern office aesthetic";
    case StyleMode::NATURE_ZEN:
        return ", zen nature art, calming natural themes, peaceful landscape elements, stress-relief aesthetic, meditative composition, serene natural beauty";

        // Specialty Room styles (NEW)
    case StyleMode::CULINARY_KITCHEN:
        return ", culinary kitchen art, food photography style, cookbook aesthetic, kitchen design elements, culinary arts composition, cooking inspiration design";
    case StyleMode::LIBRARY_ACADEMIC:
        return ", library academic art, scholarly aesthetic, book-inspired design, educational themes, intellectual atmosphere, academic institution style";
    case StyleMode::FITNESS_GYM:
        return ", fitness gym art, motivational athletic themes, dynamic energy composition, workout inspiration design, sports performance aesthetic, active lifestyle imagery";
    case StyleMode::KIDS_CARTOON:
        return ", kids cartoon art style, bright playful colors, child-friendly design, animated cartoon aesthetic, whimsical illustration, fun children's room decor";

        // Landscape styles (NEW)
    case StyleMode::PHOTOREALISTIC_LANDSCAPES:
        return ", photorealistic landscape photography, dramatic natural scenery, high detail nature scene, professional landscape photography, stunning wilderness vista";
    case StyleMode::SEASONAL_LANDSCAPES:
        return ", seasonal landscape photography, natural seasonal beauty, changing seasons atmosphere, weather-specific natural scenery, seasonal nature composition";
    case StyleMode::WEATHER_MOODS:
        return ", weather mood landscape, atmospheric weather conditions, dramatic sky formations, meteorological beauty, climate-inspired natural scenes";
    case StyleMode::TIME_OF_DAY:
        return ", time of day landscape, golden hour lighting, sunrise sunset scenery, natural lighting variations, diurnal beauty, time-specific atmosphere";

    case StyleMode::NONE:
    default:
        return "";
    }
}
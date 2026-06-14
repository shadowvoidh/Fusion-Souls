#include <SFML/Graphics.hpp>
#include "../src/FusionSouls/FusionSouls.hpp"
#include "../src/Entities/Entity.hpp"
#include "../src/CombatSystem/Combat.hpp"
#include "../src/Render/Render.hpp"

int main() {
    // Cria a janela do jogo com base no tamanho definido no seu Render.hpp
    sf::RenderWindow window(sf::VideoMode(SW, SH), "Fusion Souls - Alpha Build");
    window.setFramerateLimit(60);
    // === AQUI SE COLOCA A FONTE NO CÓDIGO ===
    sf::Font fonteJogo;
    if (!fonteJogo.loadFromFile("../resources/CinzelDecorative-Bold.ttf")) {
        // Se der erro ao carregar, o programa avisa no terminal
        std::cout << "Erro ao carregar a fonte!" << std::endl;
    }

    sf::Clock clock;

    // Inicializa o Jogador usando as estruturas que você programou
    Jogador player;
    player.pos = { SW / 2.f, SH / 2.f };
    player.fusao.aplicar({{Linhagem::Dracconico, 0.55f}, {Linhagem::Berzerker, 0.45f}});
    player.atualizarStats();

    // Cria um inimigo de teste (o sub-chefe que você codificou)
    Inimigo executor = criarExecutorDeCinzas();
    executor.pos = { SW / 2.f, SH / 4.f };

    std::vector<Inimigo> inimigos;
    inimigos.push_back(executor);

    SistemaCombate combate;
    std::vector<DanoFlutuante> danos;
    sf::Font font;
    
    // NOTA: Você vai precisar de um arquivo de fonte .ttf na pasta do projeto
    if (!font.loadFromFile("resources/arial.ttf")) {
        // Tratar erro ou usar fonte padrão do sistema
    }

    // GAME LOOP PRINCIPAL
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        sf::Event event;

        // 1. Processar Entradas do Usuário (Eventos)
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            // Aqui você vai capturar os cliques do mouse para usar as cartas
        }

        // 2. Atualizar Lógica do Jogo (Update)
        // Captura o movimento do jogador (W, A, S, D)
        sf::Vector2f movimento(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) movimento.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) movimento.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) movimento.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) movimento.x += 1.f;
        
        // Aplica velocidade básica
        player.pos += movimento * 200.f * dt;

        // Atualiza o histórico de vida para o Chronoguard e cooldowns
        player.updateSnapshot(dt);

        // 3. Renderização (Desenhar na Tela)
        window.clear(sf::Color(20, 20, 20)); // Fundo escuro Soulslike

        // Usa as suas funções do Render.hpp para desenhar o cenário e UI
        drawMundoETerreno(window, player.fusao);
        drawEntidade(window, player, player.fusao);
        
        for (auto& inimigo : inimigos) {
            drawInimigo(window, inimigo);
        }

        drawHUD(window, font, player, player.fusao);
        updateDrawDanos(window, font, danos, dt);

        window.display();
    }

    return 0;
}
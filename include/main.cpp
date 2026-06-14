#include <iostream>
#include <string>

// Estrutura que armazena as porcentagens do sistema Fusion Souls
struct FusionSouls {
    float dracconico = 0.0f;
    float berzerker = 0.0f;
    float zexcromante = 0.0f;
    float sniperVision = 0.0f;
    float chronoguard = 0.0f;
    float toxishaman = 0.0f;
    float aegisknight = 0.0f;
};

// Estrutura principal do Jogador
struct Jogador {
    std::string nome;
    int vidaMaxima = 100;
    int vidaAtual = 100;
    int estamina = 100;
    
    // Conecta o sistema de fusão ao jogador
    FusionSouls fusao;
};

int main() {
    // 1. Criando o personagem do jogador
    Jogador player;
    player.nome = "Guerreiro Inicial";
    
    // 2. Aplicando a fusão que você planejou (55% Dracônico / 45% Berzerker)
    player.fusao.dracconico = 0.55f;
    player.fusao.berzerker = 0.45f;
    
    // 3. Mostrando os dados na tela (Terminal)
    std::cout << "--- STATUS DO JOGADOR ---" << std::endl;
    std::cout << "Nome: " << player.nome << std::endl;
    std::cout << "Vida: " << player.vidaAtual << "/" << player.vidaMaxima << std::endl;
    std::cout << "Estamina: " << player.estamina << std::endl;
    
    std::cout << "\n--- LINHAGEM FUSION SOULS ---" << std::endl;
    std::cout << "Draconico: " << (player.fusao.dracconico * 100) << "%" << std::endl;
    std::cout << "Berzerker: " << (player.fusao.berzerker * 100) << "%" << std::endl;
    
    return 0;
}
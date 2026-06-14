#pragma once
// =============================================================================
//  CardSystem.hpp
//  Sistema de cartas de habilidade em tempo real.
//  Cada carta pertence a uma linhagem e tem custo, cooldown e efeito.
// =============================================================================

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <SFML/System/Vector2.hpp>
#include "FusionSouls.hpp"

// Tipo de efeito da carta
enum class EfeitoCarta {
    DanoFisico,
    DanoMagico,
    DanoVeneno,
    Cura,
    Escudo,
    InvocarEspirito,
    SlowTempo,
    DashEvasao,
    Projetil,
    AoE,          // Área de efeito
    Knockback,
    CargaFrenesi, // Berzerker acumula fúria
    RebobinarVida,// Chronoguard
    PassivaToggle,// Liga/desliga um efeito passivo
};

// Alvo da carta
enum class AlvoCarta {
    Inimigo,
    Proprio,
    Area,
    Direcao,  // O jogador mira com o mouse/joystick
};

struct Carta {
    std::string   nome;
    std::string   descricao;
    Linhagem      linhagem;       // Qual linhagem esta carta pertence
    EfeitoCarta   efeito;
    AlvoCarta     alvo;
    float         custoCooldown;  // Segundos de cooldown
    float         custoMana;      // Custo em mana
    float         custoStamina;   // Custo em estamina
    float         potencia;       // Valor base do efeito (dano, cura, etc.)
    float         duracao;        // Duração em segundos (efeitos temporários)
    float         alcance;        // Alcance em unidades do mapa
    bool          requerFusao;    // Requer threshold mínimo de fusão?
    float         fusaoMinima;    // % mínima da linhagem para usar
    int           slot;           // Slot de atalho (1-8, 0 = sem slot)

    // Estado em tempo de execução (não serializado)
    float cooldownRestante = 0.f;
    bool  ativa            = false;
};

// ---------------------------------------------------------------------------
//  Deck padrão de cartas por linhagem
// ---------------------------------------------------------------------------
inline std::vector<Carta> cartasBase() {
    return {
        // ── HUMANO ──────────────────────────────────────────────────────
        {
            "Golpe Básico", "Ataque físico simples. Sempre disponível.",
            Linhagem::Humano, EfeitoCarta::DanoFisico, AlvoCarta::Inimigo,
            0.4f, 0.f, 15.f, 1.0f, 0.f, 1.5f,
            false, 0.f, 1
        },
        {
            "Segundo Fôlego", "Recupera 20% da estamina máxima instantaneamente.",
            Linhagem::Humano, EfeitoCarta::Cura, AlvoCarta::Proprio,
            8.0f, 10.f, 0.f, 0.20f, 0.f, 0.f,
            false, 0.f, 2
        },

        // ── DRACÔNICO ───────────────────────────────────────────────────
        {
            "Sopro de Brasa", "Projétil de fogo em linha. Causa queimadura (3s).",
            Linhagem::Dracconico, EfeitoCarta::Projetil, AlvoCarta::Direcao,
            2.0f, 25.f, 10.f, 1.8f, 3.0f, 8.0f,
            true, 0.15f, 3
        },
        {
            "Voo Rasante", "Dash aéreo com invulnerabilidade (0.3s). +30% SPD após.",
            Linhagem::Dracconico, EfeitoCarta::DashEvasao, AlvoCarta::Direcao,
            5.0f, 30.f, 20.f, 0.f, 2.0f, 4.0f,
            true, 0.30f, 4
        },
        {
            "Nova Dracônica", "Explosão de fogo em área. Dano pesado ao redor.",
            Linhagem::Dracconico, EfeitoCarta::AoE, AlvoCarta::Area,
            12.0f, 70.f, 0.f, 3.5f, 0.f, 3.5f,
            true, 0.55f, 5
        },

        // ── ZEXCROMANTE ─────────────────────────────────────────────────
        {
            "Invocar Espírito", "Conjura um espírito guerreiro com espada por 10s.",
            Linhagem::Zexcromante, EfeitoCarta::InvocarEspirito, AlvoCarta::Area,
            8.0f, 50.f, 0.f, 1.2f, 10.0f, 5.0f,
            true, 0.20f, 3
        },
        {
            "Corrente Espectral", "Espíritos convergem em um inimigo: dano x3.",
            Linhagem::Zexcromante, EfeitoCarta::DanoMagico, AlvoCarta::Inimigo,
            15.0f, 80.f, 0.f, 3.0f, 0.f, 7.0f,
            true, 0.45f, 4
        },

        // ── BERZERKER ───────────────────────────────────────────────────
        {
            "Pancada Brutal", "Golpe corpo-a-corpo pesado. Knockback no inimigo.",
            Linhagem::Berzerker, EfeitoCarta::Knockback, AlvoCarta::Inimigo,
            1.5f, 0.f, 30.f, 2.5f, 0.f, 1.2f,
            true, 0.20f, 1
        },
        {
            "Fúria Imortal", "Acumula frenesi: +5% ATK por carga, até 10 cargas.",
            Linhagem::Berzerker, EfeitoCarta::CargaFrenesi, AlvoCarta::Proprio,
            0.5f, 0.f, 10.f, 0.05f, 0.f, 0.f,
            true, 0.30f, 2
        },
        {
            "Arrancada Berserker", "Dash frontal que atravessa inimigos causando dano.",
            Linhagem::Berzerker, EfeitoCarta::DashEvasao, AlvoCarta::Direcao,
            6.0f, 20.f, 40.f, 1.5f, 0.f, 5.0f,
            true, 0.40f, 3
        },

        // ── SNIPERVISIÓN ────────────────────────────────────────────────
        {
            "Tiro Perfurante", "Projétil de longo alcance. Atravessa inimigos.",
            Linhagem::SniperVision, EfeitoCarta::Projetil, AlvoCarta::Direcao,
            2.5f, 20.f, 25.f, 2.2f, 0.f, 12.0f,
            true, 0.20f, 3
        },
        {
            "Mira Crítica", "Próximo ataque: crítico garantido + dano x2.",
            Linhagem::SniperVision, EfeitoCarta::PassivaToggle, AlvoCarta::Proprio,
            10.0f, 40.f, 0.f, 2.0f, 5.0f, 0.f,
            true, 0.35f, 4
        },
        {
            "Saraivada Fantasma", "Dispara 5 projéteis em leque rapidamente.",
            Linhagem::SniperVision, EfeitoCarta::Projetil, AlvoCarta::Direcao,
            8.0f, 50.f, 30.f, 1.0f, 0.f, 10.0f,
            true, 0.50f, 5
        },

        // ── CHRONOGUARD ─────────────────────────────────────────────────
        {
            "Esquiva Temporal", "Dash com iframes de 0.5s. Tempo ao redor desacelera.",
            Linhagem::Chronoguard, EfeitoCarta::DashEvasao, AlvoCarta::Direcao,
            4.0f, 35.f, 20.f, 0.f, 1.5f, 3.0f,
            true, 0.25f, 3
        },
        {
            "Câmara Lenta", "Reduz velocidade inimiga em 40% por 4s (área).",
            Linhagem::Chronoguard, EfeitoCarta::SlowTempo, AlvoCarta::Area,
            12.0f, 60.f, 0.f, 0.40f, 4.0f, 5.0f,
            true, 0.40f, 4
        },
        {
            "Rebobinar", "Retorna a vida e posição de 3s atrás. (1 uso por batalha)",
            Linhagem::Chronoguard, EfeitoCarta::RebobinarVida, AlvoCarta::Proprio,
            60.0f, 100.f, 0.f, 3.0f, 0.f, 0.f,
            true, 0.65f, 5
        },

        // ── TOXISHAMAN ──────────────────────────────────────────────────
        {
            "Dardo Venenoso", "Projétil que aplica veneno: 8 dps por 6s.",
            Linhagem::Toxishaman, EfeitoCarta::DanoVeneno, AlvoCarta::Direcao,
            3.0f, 20.f, 10.f, 8.0f, 6.0f, 6.0f,
            true, 0.15f, 3
        },
        {
            "Nuvem de Esporos", "Lança nuvem tóxica de 3m. Inimigos dentro: lento +veneno.",
            Linhagem::Toxishaman, EfeitoCarta::AoE, AlvoCarta::Area,
            8.0f, 45.f, 0.f, 5.0f, 5.0f, 3.0f,
            true, 0.35f, 4
        },
        {
            "Metamorfose Tóxica", "Se transforma em névoa: invulnerável 2s + veneno em área.",
            Linhagem::Toxishaman, EfeitoCarta::DashEvasao, AlvoCarta::Proprio,
            15.0f, 70.f, 0.f, 15.0f, 2.0f, 3.0f,
            true, 0.55f, 5
        },

        // ── AEGISKNIGHT ─────────────────────────────────────────────────
        {
            "Golpe de Escudo", "Ataca com o escudo. Knockback + atordoa 0.5s.",
            Linhagem::Aegisknight, EfeitoCarta::Knockback, AlvoCarta::Inimigo,
            2.0f, 0.f, 35.f, 1.8f, 0.5f, 1.5f,
            true, 0.20f, 1
        },
        {
            "Barreira Colossal", "Ativa escudo: absorve 150 dano por 4s.",
            Linhagem::Aegisknight, EfeitoCarta::Escudo, AlvoCarta::Proprio,
            10.0f, 40.f, 20.f, 150.f, 4.0f, 0.f,
            true, 0.35f, 2
        },
        {
            "Onda Magnética", "Liberação de energia magnética em área. Stun 1s.",
            Linhagem::Aegisknight, EfeitoCarta::AoE, AlvoCarta::Area,
            14.0f, 60.f, 0.f, 2.8f, 1.0f, 4.0f,
            true, 0.55f, 3
        },
    };
}

// ---------------------------------------------------------------------------
//  Gerenciador do deck ativo do jogador
// ---------------------------------------------------------------------------
class DeckAtivo {
public:
    static const int MAX_SLOTS = 8;

    std::vector<Carta> cartasDisponiveis;  // Pool completo
    std::vector<Carta*> slots;             // Ponteiros para os 8 slots ativos

    DeckAtivo() {
        cartasDisponiveis = cartasBase();
        slots.resize(MAX_SLOTS, nullptr);
    }

    // Filtra cartas que o jogador pode usar com sua fusão atual
    std::vector<Carta*> cartasUsaveis(const FusionSouls& f) {
        std::vector<Carta*> resultado;
        for (auto& c : cartasDisponiveis) {
            if (!c.requerFusao || f.get(c.linhagem) >= c.fusaoMinima)
                resultado.push_back(&c);
        }
        return resultado;
    }

    // Atualiza cooldowns
    void update(float dt) {
        for (auto& c : cartasDisponiveis)
            if (c.cooldownRestante > 0.f)
                c.cooldownRestante = std::max(0.f, c.cooldownRestante - dt);
    }

    // Tenta usar uma carta (retorna false se em cooldown ou sem recursos)
    bool usarCarta(Carta& c, float& manaAtual, float& staminaAtual) {
        if (c.cooldownRestante > 0.f) return false;
        if (manaAtual < c.custoMana) return false;
        if (staminaAtual < c.custoStamina) return false;

        manaAtual    -= c.custoMana;
        staminaAtual -= c.custoStamina;
        c.cooldownRestante = c.custoCooldown;
        return true;
    }

    // Equipa uma carta em um slot específico
    void equiparSlot(int slot, Carta* c) {
        if (slot >= 0 && slot < MAX_SLOTS)
            slots[slot] = c;
    }

    Carta* getSlot(int slot) {
        if (slot >= 0 && slot < MAX_SLOTS) return slots[slot];
        return nullptr;
    }
};
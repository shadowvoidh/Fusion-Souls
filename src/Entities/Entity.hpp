#pragma once
// =============================================================================
//  Entity.hpp
//  Entidade base + Player + Enemy com máquina de estados para IA de chefes.
// =============================================================================

#include <string>
#include <vector>
#include <functional>
#include <deque>
#include <SFML/System/Vector2.hpp>
#include "FusionSouls.hpp"
#include "CardSystem.hpp"

// ---------------------------------------------------------------------------
//  Status Effect (veneno, slow, queimadura, etc.)
// ---------------------------------------------------------------------------
enum class TipoStatus {
    Nenhum,
    Veneno,
    Queimadura,
    Paralisia,
    Lento,
    Atordoado,
    Frenesi,
};

struct StatusEffect {
    TipoStatus tipo     = TipoStatus::Nenhum;
    float      duracao  = 0.f;   // segundos restantes
    float      potencia = 0.f;   // dps, % slow, etc.
    bool       ativo    = false;
};

// ---------------------------------------------------------------------------
//  Entidade Base
// ---------------------------------------------------------------------------
struct EntidadeBase {
    // Identidade
    std::string nome;
    int         nivel = 1;

    // Posição e movimento
    sf::Vector2f pos      = {0.f, 0.f};
    sf::Vector2f vel      = {0.f, 0.f};
    sf::Vector2f dir      = {1.f, 0.f};  // direção normalizada para onde olha
    float        rotacao  = 0.f;

    // Stats atuais (derivados do Fusion Souls ou fixos para inimigos)
    Stats stats;

    // Recursos atuais
    float hpAtual      = 0.f;
    float manaAtual    = 0.f;
    float staminaAtual = 0.f;
    float shieldAtual  = 0.f;  // HP do escudo (Aegisknight)

    // Status effects ativos
    std::vector<StatusEffect> statusAtivos;

    // Flags
    bool  vivo         = true;
    bool  invulneravel = false;
    float iframeTimer  = 0.f;  // Segundos de invulnerabilidade

    // ── Helpers ────────────────────────────────────────────────────────

    void inicializarRecursos() {
        hpAtual      = stats.hp;
        manaAtual    = stats.mana;
        staminaAtual = stats.stamina;
        shieldAtual  = stats.shieldHP;
    }

    // Recebe dano com cálculo de defesa, escudo e resistências
    float receberDano(float dano, bool magico, bool fogo = false) {
        if (!vivo || invulneravel || iframeTimer > 0.f) return 0.f;

        // Resistência a fogo (Dracônico)
        if (fogo) dano *= (1.f - stats.fireResist);

        // Redução por defesa
        float reducao = magico ? stats.defMagica : stats.def;
        float danoBruto = std::max(1.f, dano - reducao * 0.3f); // Defesa reduz 30% do valor

        // Escudo absorve primeiro
        if (shieldAtual > 0.f) {
            float absorvido = std::min(shieldAtual, danoBruto);
            shieldAtual -= absorvido;
            danoBruto   -= absorvido;
        }

        hpAtual -= danoBruto;
        if (hpAtual <= 0.f) {
            hpAtual = 0.f;
            vivo    = false;
        }

        iframeTimer = 0.12f;  // Mini-iframes após hit
        return danoBruto;
    }

    // Cura HP
    void curar(float valor) {
        hpAtual = std::min(stats.hp, hpAtual + valor);
    }

    // Aplica/atualiza status effects
    void aplicarStatus(TipoStatus tipo, float dur, float pot) {
        // Atualiza se já existe, caso contrário adiciona
        for (auto& s : statusAtivos) {
            if (s.tipo == tipo) {
                s.duracao  = std::max(s.duracao, dur);
                s.potencia = std::max(s.potencia, pot);
                s.ativo    = true;
                return;
            }
        }
        statusAtivos.push_back({tipo, dur, pot, true});
    }

    void updateStatus(float dt) {
        float danoPorTick = 0.f;
        for (auto& s : statusAtivos) {
            if (!s.ativo) continue;
            s.duracao -= dt;
            if (s.tipo == TipoStatus::Veneno || s.tipo == TipoStatus::Queimadura)
                danoPorTick += s.potencia * dt;
            if (s.duracao <= 0.f) s.ativo = false;
        }
        // Remove inativos
        statusAtivos.erase(
            std::remove_if(statusAtivos.begin(), statusAtivos.end(),
                [](const StatusEffect& s){ return !s.ativo; }),
            statusAtivos.end());

        if (danoPorTick > 0.f && vivo)
            receberDano(danoPorTick, true);

        if (iframeTimer > 0.f)
            iframeTimer = std::max(0.f, iframeTimer - dt);
    }

    bool temStatus(TipoStatus t) const {
        for (const auto& s : statusAtivos)
            if (s.tipo == t && s.ativo) return true;
        return false;
    }

    float pctHP()   const { return stats.hp   > 0 ? hpAtual   / stats.hp   : 0.f; }
    float pctMana() const { return stats.mana > 0 ? manaAtual / stats.mana : 0.f; }
};

// ---------------------------------------------------------------------------
//  Jogador
// ---------------------------------------------------------------------------
struct Jogador : EntidadeBase {
    FusionSouls fusao;
    DeckAtivo   deck;
    int         experiencia     = 0;
    int         xpParaProxNivel = 100;

    // Aliados escolhidos
    std::string aliadoSlot1;  // "Valle" ou "Brutosdokker"
    std::string aliadoSlot2;  // "Slowex" ou "Jumppex"

    // Estado de movimento
    float dashTimer    = 0.f;
    bool  dashAtivo    = false;
    sf::Vector2f dashDir = {0.f, 0.f};

    // Chronoguard: histórico de vida para rebobinar
    struct SnapshotVida {
        float hp, mana, stamina;
        sf::Vector2f pos;
    };
    std::deque<SnapshotVida> historicoVida;  // Últimos 3s de snapshots

    // Berzerker: cargas de fúria
    int   cargasFuria    = 0;
    int   maxCargasFuria = 10;
    float bonusFuria     = 0.f;  // ATK extra acumulado

    // Para evolução da aparência visual baseada na fusão
    sf::Color   corPrimaria   = sf::Color(200, 180, 160);
    sf::Color   corSecundaria = sf::Color(80, 80, 100);
    float       escalaVisual  = 1.0f;
    bool        temAsas       = false;   // Dracônico >= 40%
    bool        temEscudo     = false;   // Aegisknight >= 35%
    int         numEspiritos  = 0;       // Zexcromante ativo

    // Aplica nova fusão e recalcula tudo
    void aplicarFusao(std::vector<std::pair<Linhagem, float>> mix) {
        fusao.aplicar(mix);
        stats         = calcularStats(fusao, nivel);
        atualizarAparencia();
        // Mantém proporção dos recursos ao mudar fusão
        float pctHP    = hpAtual / std::max(1.f, stats.hp);
        float pctMana  = manaAtual / std::max(1.f, stats.mana);
        float pctStam  = staminaAtual / std::max(1.f, stats.stamina);
        hpAtual       = stats.hp      * pctHP;
        manaAtual     = stats.mana    * pctMana;
        staminaAtual  = stats.stamina * pctStam;
        shieldAtual   = stats.shieldHP;
    }

    void atualizarAparencia() {
        float pDraco   = fusao.get(Linhagem::Dracconico);
        float pBerz    = fusao.get(Linhagem::Berzerker);
        float pAegis   = fusao.get(Linhagem::Aegisknight);
        float pChron   = fusao.get(Linhagem::Chronoguard);
        float pToxi    = fusao.get(Linhagem::Toxishaman);
        float pZex     = fusao.get(Linhagem::Zexcromante);
        float pSniper  = fusao.get(Linhagem::SniperVision);

        // Cor baseada na linhagem dominante
        Linhagem dom = fusao.dominante();
        switch(dom) {
            case Linhagem::Dracconico:
                corPrimaria   = sf::Color(220, 80, 40);   // Vermelho-fogo
                corSecundaria = sf::Color(255, 160, 50);  // Laranja
                break;
            case Linhagem::Berzerker:
                corPrimaria   = sf::Color(160, 30, 30);   // Vermelho escuro
                corSecundaria = sf::Color(80, 20, 20);
                break;
            case Linhagem::Zexcromante:
                corPrimaria   = sf::Color(130, 60, 200);  // Roxo espectral
                corSecundaria = sf::Color(200, 200, 255);
                break;
            case Linhagem::SniperVision:
                corPrimaria   = sf::Color(50, 130, 180);  // Azul-aço
                corSecundaria = sf::Color(200, 220, 240);
                break;
            case Linhagem::Chronoguard:
                corPrimaria   = sf::Color(100, 200, 220); // Ciano temporal
                corSecundaria = sf::Color(200, 240, 255);
                break;
            case Linhagem::Toxishaman:
                corPrimaria   = sf::Color(80, 180, 60);   // Verde veneno
                corSecundaria = sf::Color(180, 240, 60);
                break;
            case Linhagem::Aegisknight:
                corPrimaria   = sf::Color(180, 180, 200); // Prata
                corSecundaria = sf::Color(100, 120, 200); // Azul-aço
                break;
            default:
                corPrimaria   = sf::Color(200, 180, 160); // Bege humano
                corSecundaria = sf::Color(80, 80, 100);
                break;
        }

        // Modificações visuais por threshold
        temAsas       = pDraco  >= 0.40f;
        temEscudo     = pAegis  >= 0.35f;
        numEspiritos  = (int)stats.spiritCount;
        escalaVisual  = 1.0f + pBerz * 0.30f;  // Berzerker = mais grandão
    }

    // Registra snapshot para Chronoguard
    void registrarSnapshot() {
        historicoVida.push_back({hpAtual, manaAtual, staminaAtual, pos});
        // Mantém apenas ~3 segundos (assumindo 60fps = 180 frames, 10 snapshots/s = 30)
        if (historicoVida.size() > 30) historicoVida.pop_front();
    }

    // Reverte 3 segundos atrás (Rebobinar)
    bool rebobinar() {
        if (historicoVida.empty()) return false;
        const auto& snap = historicoVida.front();
        hpAtual      = snap.hp;
        manaAtual    = snap.mana;
        staminaAtual = snap.stamina;
        pos          = snap.pos;
        vivo         = (hpAtual > 0.f);
        historicoVida.clear();
        return true;
    }

    // Ganho de experiência e level up
    bool ganharXP(int xp) {
        experiencia += xp;
        if (experiencia >= xpParaProxNivel) {
            experiencia    -= xpParaProxNivel;
            nivel          += 1;
            xpParaProxNivel = 100 + nivel * 50;
            stats           = calcularStats(fusao, nivel);
            // HP sobe com nível, mantém proporção
            hpAtual  = std::min(hpAtual + 30.f, stats.hp);
            manaAtual = std::min(manaAtual + 20.f, stats.mana);
            return true;
        }
        return false;
    }
};

// ---------------------------------------------------------------------------
//  Inimigo / Chefe — Máquina de Estados
// ---------------------------------------------------------------------------
enum class EstadoIA {
    Patrulha,
    Alerta,
    Perseguir,
    AtaqueNormal,
    AtaqueEspecial,
    Fase2,         // Chefes mudam de fase abaixo de 50% HP
    Recuar,        // Cria distância para recuperar
    Atordoado,
    Morto,
};

struct Fase {
    float         hpThreshold;     // % de HP para ativar esta fase
    std::string   descricao;
    float         multiplicadorAtk;
    float         multiplicadorSpd;
    float         cooldownEspecial; // CD do ataque especial nesta fase
    sf::Color     corFase;          // Cor do chefe nesta fase
};

struct AtaqueEspecialDef {
    std::string  nome;
    float        cooldown;
    float        cooldownRestante = 0.f;
    float        dano;
    float        alcance;
    EfeitoCarta  tipo;
    float        duracaoStatus;  // 0 = sem status
    TipoStatus   status;
    std::function<void(EntidadeBase&)> executar;  // lógica custom
};

struct Inimigo : EntidadeBase {
    // Tipo
    bool         eChefe         = false;
    bool         eSubChefe      = false;
    FusionSouls  fusaoInimigo;  // Chefes também têm linhagens

    // IA
    EstadoIA          estado          = EstadoIA::Patrulha;
    float             timerEstado     = 0.f;
    float             timerAtaque     = 0.f;
    float             alcanceDeteccao = 200.f;
    float             alcanceAtaque   = 50.f;
    sf::Vector2f      posPatrulha     = {0.f, 0.f};
    int               faseAtual       = 0;
    std::vector<Fase> fases;
    std::vector<AtaqueEspecialDef> ataquesEspeciais;

    // Drops
    int   xpRecompensa  = 10;
    float essenciaRecom = 0.f;   // Quantidade de essência de fusão dropada

    // Inicializa um chefe simples
    void configurarChefe(const std::string& n, int lv,
                         const FusionSouls& f, bool subChefe = false)
    {
        nome      = n;
        nivel     = lv;
        eChefe    = !subChefe;
        eSubChefe = subChefe;
        fusaoInimigo = f;

        // Stats do chefe são mais fortes (2x base)
        stats = calcularStats(fusaoInimigo, nivel);
        stats.hp  *= (subChefe ? 1.8f : 2.5f);
        stats.atk *= (subChefe ? 1.5f : 2.0f);
        stats.def *= 1.3f;
        inicializarRecursos();

        xpRecompensa = nivel * (subChefe ? 80 : 200);
        essenciaRecom = subChefe ? 0.15f : 0.35f;
    }

    // Verifica troca de fase baseado em % HP
    bool verificarFase() {
        if (faseAtual + 1 >= (int)fases.size()) return false;
        if (pctHP() <= fases[faseAtual + 1].hpThreshold) {
            faseAtual++;
            estado = EstadoIA::Fase2;
            timerEstado = 2.0f;  // Pausa dramática na troca de fase
            return true;
        }
        return false;
    }

    // Atualização da IA (simplificada — Motor usa isso)
    EstadoIA updateIA(const sf::Vector2f& posJogador, float dt) {
        if (!vivo) return EstadoIA::Morto;

        float distJog = std::hypot(posJogador.x - pos.x, posJogador.y - pos.y);
        timerEstado  -= dt;
        timerAtaque  -= dt;

        // Atualiza cooldowns dos especiais
        for (auto& ae : ataquesEspeciais)
            if (ae.cooldownRestante > 0.f)
                ae.cooldownRestante -= dt;

        switch(estado) {
            case EstadoIA::Patrulha:
                if (distJog < alcanceDeteccao) {
                    estado      = EstadoIA::Alerta;
                    timerEstado = 0.5f;
                }
                break;

            case EstadoIA::Alerta:
                if (timerEstado <= 0.f)
                    estado = EstadoIA::Perseguir;
                break;

            case EstadoIA::Perseguir:
                if (distJog <= alcanceAtaque) {
                    estado = EstadoIA::AtaqueNormal;
                    timerAtaque = 1.0f;
                } else if (distJog > alcanceDeteccao * 1.5f) {
                    estado = EstadoIA::Patrulha;
                }
                // Decide usar ataque especial
                if (eChefe || eSubChefe) {
                    for (auto& ae : ataquesEspeciais) {
                        if (ae.cooldownRestante <= 0.f && distJog <= ae.alcance) {
                            estado = EstadoIA::AtaqueEspecial;
                            break;
                        }
                    }
                }
                break;

            case EstadoIA::AtaqueNormal:
                if (timerAtaque <= 0.f)
                    estado = EstadoIA::Perseguir;
                break;

            case EstadoIA::AtaqueEspecial:
                if (timerEstado <= 0.f)
                    estado = EstadoIA::Perseguir;
                break;

            case EstadoIA::Fase2:
                if (timerEstado <= 0.f) {
                    estado = EstadoIA::Perseguir;
                    // Aplica buffs da nova fase
                    if (faseAtual < (int)fases.size()) {
                        stats.atk *= fases[faseAtual].multiplicadorAtk;
                        stats.spd *= fases[faseAtual].multiplicadorSpd;
                    }
                }
                break;

            case EstadoIA::Atordoado:
                if (timerEstado <= 0.f)
                    estado = EstadoIA::Perseguir;
                break;

            default: break;
        }

        return estado;
    }

    // Velocidade real considerando slow temporal e status
    float velReal() const {
        float v = stats.spd;
        for (const auto& s : statusAtivos)
            if (s.tipo == TipoStatus::Lento && s.ativo)
                v *= (1.f - std::min(0.9f, s.potencia));
        if (temStatus(TipoStatus::Paralisia)) v = 0.f;
        return v;
    }
};

// ---------------------------------------------------------------------------
//  Fábrica de chefes do Mapa 1: Inferno da Guerra
// ---------------------------------------------------------------------------
inline Inimigo criarMonarkGonter() {
    Inimigo boss;
    FusionSouls f;
    f.aplicar({{Linhagem::Dracconico, 1.0f}});
    boss.configurarChefe("Monark Gonter", 15, f, false);
    boss.alcanceDeteccao = 350.f;
    boss.alcanceAtaque   = 80.f;

    // Fases
    boss.fases = {
        {1.00f, "Fase 1: Domínio Aéreo",       1.0f, 1.0f, 6.0f,  sf::Color(220, 80, 40)},
        {0.50f, "Fase 2: Fúria Dracônica",      1.4f, 1.3f, 3.5f,  sf::Color(255, 40,  0)},
        {0.20f, "Fase 3: Ascensão do Dragão",   1.8f, 1.6f, 2.0f,  sf::Color(255, 180, 0)},
    };

    // Ataques especiais
    boss.ataquesEspeciais = {
        {
            "Rajada de Fogo", 5.0f, 0.f, 80.f, 7.0f, EfeitoCarta::Projetil, 3.0f, TipoStatus::Queimadura,
            [](EntidadeBase& alvo) { alvo.aplicarStatus(TipoStatus::Queimadura, 3.0f, 15.f); }
        },
        {
            "Mergulho Flamejante", 10.0f, 0.f, 120.f, 5.0f, EfeitoCarta::AoE, 0.f, TipoStatus::Nenhum,
            [](EntidadeBase& alvo) { /* knockback implementado no motor */ }
        },
        {
            "Escamas Dracônicas", 15.0f, 0.f, 0.f, 0.f, EfeitoCarta::PassivaToggle, 0.f, TipoStatus::Nenhum,
            [](EntidadeBase& self) { self.invulneravel = true; /* 2s de invuln */ }
        },
    };

    boss.xpRecompensa = 500;
    boss.essenciaRecom = 0.40f;
    return boss;
}

inline Inimigo criarExecutorDeCinzas() {
    Inimigo sub;
    FusionSouls f;
    f.aplicar({{Linhagem::Berzerker, 0.70f}, {Linhagem::Dracconico, 0.30f}});
    sub.configurarChefe("Executor de Cinzas", 10, f, true);
    sub.alcanceDeteccao = 280.f;
    sub.alcanceAtaque   = 60.f;

    sub.fases = {
        {1.00f, "Fase 1: Executor",         1.0f, 1.0f, 7.0f, sf::Color(160, 40, 20)},
        {0.40f, "Fase 2: Derradeiro Furor", 1.6f, 1.2f, 4.0f, sf::Color(255, 80, 0)},
    };

    sub.ataquesEspeciais = {
        {
            "Triturar", 4.0f, 0.f, 90.f, 45.f, EfeitoCarta::DanoFisico, 0.f, TipoStatus::Nenhum,
            [](EntidadeBase& alvo) { /* Golpe em área de 45px */ }
        },
        {
            "Escamas de Lava", 12.0f, 0.f, 60.f, 80.f, EfeitoCarta::AoE, 4.0f, TipoStatus::Queimadura,
            [](EntidadeBase& alvo) { alvo.aplicarStatus(TipoStatus::Queimadura, 4.0f, 20.f); }
        },
    };

    sub.xpRecompensa = 200;
    sub.essenciaRecom = 0.20f;
    return sub;
}
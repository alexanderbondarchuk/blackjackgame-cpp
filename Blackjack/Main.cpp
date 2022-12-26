#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
using namespace std;

class Card
{
public:
	enum rank
	{
		ACE = 1,
		TWO,
		THREE,
		FOUR,
		FIVE,
		SIX,
		SEVEN,
		EIGHT,
		NINE,
		TEN,
		JACK,
		QUEEN,
		KING
	};
	enum suit
	{
		CLUBS,
		DIAMONDS,
		HEARTS,
		SPADES
	};
	friend ostream& operator<<(ostream& os, const Card& aCard);
	Card(rank r = ACE, suit s = SPADES, bool ifu = true);

	// Значение карты
	int GetValue() const;

	// Переворачивает карты
	void Flip();

private:
	rank m_Rank;
	suit m_Suit;
	bool m_IsFaceUp;
};

Card::Card(rank r, suit s, bool ifu): m_Rank(r),
m_Suit(s),
m_IsFaceUp(ifu) {}

int Card::GetValue() const
{
	int value = 0;

	if (m_IsFaceUp)
	{
		value = m_Rank;

		if (value > 10)
		{
			value = 10;
		}
	}

	return value;
}

void Card::Flip()
{
	m_IsFaceUp = !m_IsFaceUp;
}

class Hand
{
public:
	Hand();
	virtual ~Hand();

	// Добавляет карту в руку
	void Add(Card* pCard);

	// Очищает руку от карт
	void Clear();

	// Получает сумму очков карт в руке,
	// присваивая тузу значение 1 или 11 в зависимости от ситуации
	int GetTotal() const;

protected:
	vector<Card*> m_Cards;
};

Hand::Hand()
{
	m_Cards.reserve(7);
}

Hand::~Hand()
{
	Clear();
}

void Hand::Add(Card* pCard)
{
	m_Cards.push_back(pCard);
}

void Hand::Clear()
{
	// проходит по вектору, освобождая всю память в куче
	vector<Card*>::iterator iter = m_Cards.begin();

	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
	{
		// освобождает память в куче
		delete* iter;
		// обнуляет указатель, чтобы он не указывал в несуществующую область памяти
		*iter = 0;
	}

	// очищает вектор указателей
	m_Cards.clear();
}

int Hand::GetTotal() const
{
	// если карт на руке нет
	if (m_Cards.empty())
	{
		return 0;
	}

	// если первая карта лежит рубашкой вверх
	if (m_Cards[0]->GetValue() == 0)
	{
		return 0;
	}

	int total = 0;
	vector<Card*>::const_iterator iter;

	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
	{
		total += (*iter)->GetValue();
	}

	// определяет, держит ли рука туз
	bool containsAce = false;
	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
	{
		if ((*iter)->GetValue() == Card::ACE)
		{
			containsAce = true;
		}
	}

	// если рука держит туз и сумма достаточно мала, туз даёт 11 очков
	if (containsAce && total <= 11)
	{
		// добавляется 10 очков, т.к. тузы ранее были учтены как 1 очко
		total += 10;
	}

	return total;
}

class GenericPlayer: public Hand
{
	friend ostream& operator<<(ostream& os, const GenericPlayer& aGenericPlayer);

public:
	GenericPlayer(const string& name = "");
	virtual ~GenericPlayer();

	// Показывает, хочет ли игрок продолжать брать карты
	virtual bool IsHitting() const = 0;

	// Возвращает значение, если игрок имеет перебор - сумму очков больше 21
	bool IsBusted() const;

	// Объявляет, что игрок имеет перебор
	void Bust() const;

protected:
	string m_Name;
};

GenericPlayer::GenericPlayer(const string& name): m_Name(name) {}

GenericPlayer::~GenericPlayer() {}

bool GenericPlayer::IsBusted() const
{
	return GetTotal() > 21;
}

void GenericPlayer::Bust() const
{
	cout << m_Name << " busts." << endl;
}

class Player: public GenericPlayer
{
public:
	Player(const string& name = "");
	virtual ~Player();

	// показывает, хочет ли игрок продолжать брать карты
	virtual bool IsHitting() const;

	// объявляет, что игрок победил
	void Win() const;

	// объявляет, что игрок проиграл
	void Lose() const;

	// объявляет ничью
	void Push() const;
};

Player::Player(const string& name): GenericPlayer(name) {}

Player::~Player() {}

bool Player::IsHitting() const
{
	cout << m_Name << ", do you want a hit? (Y/N)";
	char response;
	cin >> response;

	return response == 'y' || response == 'Y';
}

void Player::Win() const
{
	cout << m_Name << " wins." << endl;
}

void Player::Lose() const
{
	cout << m_Name << " loses." << endl;
}

void Player::Push() const
{
	cout << m_Name << " pushes." << endl;
}

class House: public GenericPlayer
{
public:
	House(const string& name = "House");
	virtual ~House();
	// показывает хочет ли игрок продолжать брать карты
	virtual bool IsHitting() const;
	// переворачивает первую карту
	void FlipFirstCard();
};

House::House(const string& name): GenericPlayer(name) {}

House::~House() {}

bool House::IsHitting() const
{
	return GetTotal() <= 16;
}

void House::FlipFirstCard()
{
	if (!m_Cards.empty())
	{
		m_Cards[0]->Flip();
	}
	else
	{
		cout << "No card to flip!" << endl;
	}
}

class Deck: public Hand
{
public:
	Deck();
	virtual ~Deck();

	// создаёт стандартную колоду из 52 карт
	void Populate();

	// тасует карты
	void Shuffle();

	// раздаёт одну карту в руку
	void Deal(Hand& aHand);

	// даёт дополнительные карты игроку
	void AdditionalCards(GenericPlayer& aGenericPlayer);
};

Deck::Deck() {
	m_Cards.reserve(52);
	Populate();
}

Deck::~Deck() {}

void Deck::Populate() {
	Clear();
	// создаёт стандартную колоду
	for (int s = Card::CLUBS; s <= Card::SPADES; ++s) {
		for (int r = Card::ACE; r <= Card::KING; ++r)
		{
			// при добавлении карт используется преобразование типов из int в типы-перечисления
			Add(new Card(static_cast<Card::rank>(r), static_cast<Card::suit>(s)));
		}
	}
}

void Deck::Shuffle() {
	random_device rd;
	mt19937 g(rd());

	shuffle(m_Cards.begin(), m_Cards.end(), g);
}

void Deck::Deal(Hand& aHand) {
	if (!m_Cards.empty()) {
		aHand.Add(m_Cards.back());
		m_Cards.pop_back();
	}
	else {
		cout << "Out of cards. Unable to deal.";
	}
}

void Deck::AdditionalCards(GenericPlayer& aGenericPlayer) {
	cout << endl;

	// продолжает раздавать карты до тех пор, пока у игрока 
	// не случается перебор или пока он не хочет взять ещё одну карту
	while (!aGenericPlayer.IsBusted() && aGenericPlayer.IsHitting()) {
		Deal(aGenericPlayer);
		cout << aGenericPlayer << endl;
		if (aGenericPlayer.IsBusted()) {
			aGenericPlayer.Bust();
		}
	}
}

class Game {
public:
	Game(const vector<string>& names);
	~Game();
	// проводит игру в Blackjack
	void Play();
private:
	Deck m_Deck;
	House m_House;
	vector<Player> m_Players;
};

Game::Game(const vector<string>& names) {
	// создаёт вектор игроков из вектора с именами
	vector<string>::const_iterator pName;

	for (pName = names.begin(); pName != names.end(); ++pName) {
		m_Players.push_back(Player(*pName));
	}

	// засевает генератор случайных чисел
	// todo нужно ли засевать, если внутри Shuffle используется свой генератор случайных чисел?
	srand(static_cast<unsigned int>(time(0)));
	m_Deck.Populate();
	m_Deck.Shuffle();
}

Game::~Game() {}

void Game::Play() {
	// раздаёт каждому по две стартовые карты
	vector<Player>::iterator pPlayer;
	for (int i = 0; i < 2; ++i) {
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
			m_Deck.Deal(*pPlayer);
		}
		m_Deck.Deal(m_House);
	}

	// прячет первую карту дилера
	m_House.FlipFirstCard();

	// открывает руки всех игроков
	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
		cout << *pPlayer << endl;
	}

	cout << m_House << endl;

	// раздаёт игрокам дополнительные карты
	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
		m_Deck.AdditionalCards(*pPlayer);
	}

	// показывает первую карту дилера
	m_House.FlipFirstCard();
	cout << endl << m_House;

	// раздаёт дилеру дополнительные карты
	m_Deck.AdditionalCards(m_House);

	if (m_House.IsBusted()) {
		// все, кто остался в игре, побеждают
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
			if (!pPlayer->IsBusted()) {
				pPlayer->Win();
			}
		}
	}
	else {
		// сравнивает суммы всех оставшихся игроков с суммой очков дилера
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
			if (!pPlayer->IsBusted()) {
				if (pPlayer->GetTotal() > m_House.GetTotal()) {
					pPlayer->Win();
				}
				else if (pPlayer->GetTotal() < m_House.GetTotal()) {
					pPlayer->Lose();
				}
				else {
					pPlayer->Push();
				}
			}
		}
	}

	// очищает руки всех игроков
	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
		pPlayer->Clear();
	}

	m_House.Clear();
}

ostream& operator<<(ostream& os, const Card& aCard);
ostream& operator<<(ostream& os, const GenericPlayer& aGenericPlayer);

// перегружает оператор <<, чтобы получить возможность отправить
// объект типа Card в поток cout
ostream& operator<<(ostream& os, const Card& aCard) {
	const string RANKS[] = { "0", "A", "2", "3", "4", "5","6", "7", "8", "9", "10", "J", "Q", "K" };
	const string SUITS[] = { "c", "d", "h", "s" };

	if (aCard.m_IsFaceUp) {
		os << RANKS[aCard.m_Rank] << SUITS[aCard.m_Suit];
	}
	else {
		os << "XX";
	}

	return os;
}

// перегружает оператор <<, чтобы получить возможность отправить
// объект типа GenericPlayer в поток cout
ostream& operator<<(ostream& os, const GenericPlayer& aGenericPlayer) {
	os << aGenericPlayer.m_Name << ":\t";
	vector<Card*>::const_iterator pCard;

	if (!aGenericPlayer.m_Cards.empty()) {
		for (pCard = aGenericPlayer.m_Cards.begin(); pCard != aGenericPlayer.m_Cards.end(); ++pCard) {
			os << *(*pCard) << "\t";
		}
		if (aGenericPlayer.GetTotal() != 0) {
			cout << "(" << aGenericPlayer.GetTotal() << ")";
		}
	}
	else {
		os << "<empty>";
	}

	return os;
}

int main()
{
	cout << "\t\tWelcome to Blackjack!" << endl << endl;
	int numPlayers = 0;
	while (numPlayers < 1 || numPlayers > 7) {
		cout << "How many Players? (1-7): ";
		cin >> numPlayers;
	}

	vector<string> names;
	string name;
	for (int i = 0; i < numPlayers; ++i) {
		cout << "Enter player name: ";
		cin >> name;
		names.push_back(name);
	}
	cout << endl;

	// игровой цикл
	Game aGame(names);
	char again = 'y';
	while (again != 'n' && again != 'N') {
		aGame.Play();
		cout << endl << "Do you want to play again? (Y/N): ";
		cin >> again;
	}

	return 0;
}
import csv, random

random.seed(0)

categories = [i for i in range(20)]
user_categories = [random.choices(categories, k=3) for i in range(2000)]
user_questions = []
views = [0] * 1000
ratings = [0] * 1000
answer_counts = [0] * 1000
user_answers = []
questions_hidden = random.sample(range(1000), 100)
for user in range(2000):
    count = random.randint(20, 50)
    possible_questions = [cat * 50 + i for i in range(50) for cat in user_categories[user]]
    my_views = random.sample(possible_questions, count)
    my_votes = random.choices([-1, 0, 1], k=count, weights=[2, 6, 2])
    my_data = list(zip(my_views, my_votes))
    for i, vote in my_data:
        views[i] += 1
        ratings[i] += vote
    user_questions.append(my_data)
    my_answers = random.sample(possible_questions, 5)
    for i in my_answers:
        answer_counts[i] += 1
    user_answers.append(my_answers)

with open('test_data/questions.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for i in range(1000):
        writer.writerow([f'id{i}', f'question{i}', f'Question text {i}', views[i], ratings[i], answer_counts[i]])

with open('test_data/users.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for i in range(2000):
        writer.writerow([f'userBig{i}'])

with open('test_data/users_questions.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for user in range(2000):
        for question, vote in user_questions[user]:
            writer.writerow([f'userBig{user}', f'id{question}', vote])

with open('test_data/answers.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for user in range(2000):
        for question in user_answers[user]:
            writer.writerow([f'userBig{user}', f'id{question}'])

with open('test_data/question_authors.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for question in range(1000):
        user = question % 100
        writer.writerow([f'userBig{user}', f'id{question}'])

with open('test_data/questions_hidden.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for question in questions_hidden:
        writer.writerow([f'id{question}'])
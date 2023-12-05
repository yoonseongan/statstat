class Student:
    def __init__(self, name, computer_score):
        self.name = name
        self.computer_score = computer_score

    def get_name(self):
        return self.name

    def get_average_score(self):
        return self.computer_score

    def edit_computer_score(self, new_score):
        self.computer_score = new_score

class NaturalScience(Student):
    def __init__(self, name, computer_score, science_score):
        super().__init__(name, computer_score)
        self.science_score = science_score

    def get_average_score(self):
        return (self.computer_score + self.science_score) / 2

    def edit_science_score(self, new_score):
        self.science_score = new_score

class LiberalArts(Student):
    def __init__(self, name, computer_score, sociology_score):
        super().__init__(name, computer_score)
        self.sociology_score = sociology_score

    def get_average_score(self):
        return (self.computer_score + self.sociology_score) / 2

    def edit_sociology_score(self, new_score):
        self.sociology_score = new_score

class FirstGradeStudent(Student):
    def __init__(self, name, computer_score, science_score, sociology_score):
        super().__init__(name, computer_score)
        self.science_score = science_score
        self.sociology_score = sociology_score

    def get_average_score(self):
        return (self.computer_score + self.science_score + self.sociology_score) / 3

    def edit_science_score(self, new_score):
        self.science_score = new_score

    def edit_sociology_score(self, new_score):
        self.sociology_score = new_score


student = FirstGradeStudent("안윤성", 0, 0, 0) #점수 초기화



new_computer_score = int(input("컴퓨터 점수 : "))
student.edit_computer_score(new_computer_score)

new_science_score = int(input("과학 점수 : "))
student.edit_science_score(new_science_score)

new_sociology_score = int(input("사회 점수 : "))
student.edit_sociology_score(new_sociology_score)


print(f"{student.get_name()}의 평균 점수: {student.get_average_score()}")

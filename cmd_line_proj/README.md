CSCI 347--Computer Systems II
=============================
(4 credit hours)

Western Washington University

Fall 2019

Instructor:    Aran Clauson

Office:        CF 411

Office Hours:  10T, 9RF

Email:         Aran.Clauson@wwu.edu

Phone:         (360) 650-3819

#Course Description

Modern software is a coordinated effort between the application logic stored in the
program and services provided by the operating system. This course focuses on how
a program uses the OS services. Topics include virtual memory, dynamic memory,
System I/O, process management, and concurrency concepts including interprocess
and multi-threaded programming.

**Prerequisites:** CSCI 247.

# Meeting Times

|CRN | Lecture  |
|----|----------|
|42481 | ES 027 MTWF 2:00-2:50 |

# Communication

I am committed to establishing and maintaining a classroom climate that is
inclusive and respectful for all students. Learning includes being able to voice
a variety of perspectives, and classroom discussion is encouraged. While
expressed ideas may vary or be opposed to one another, it is important for all
of us to listen and engage respectfully with each other.

We will use Canvas to post some assignments and grades, but please **do not**
use Canvas conversations.  I will not check Canvas for communication.  Email is
much faster and is preferred.

CS Support has built a survival guide to help navigate the department's
environment, available at the following URL:

https://support.cs.wwu.edu/index.php/CS_Technical_Survival_Guide

# Texts

"Advanced Programming in the UNIX Environment" 3rd edition, by Stevens and Rago
(978-0321637734).

# Outcomes

On successful completion of the course, students will demonstrate:
* Thorough understanding of development in the UNIX environment
 ** Ability to design and implement substantial software projects in the C
    programming language
 ** Basic understanding of shell programming, UNIX development tools, and system
    utilities.
* Basic understanding of operating systems including file systems, resources,
  and system-calls
* Basic understanding of interprocess communication including pipes, shared
  memory, and messages.
* Strong understanding of problems and techinques in concurrent programming
* Thorough understanding of the purpose and use of semaphores, monitors, and
  rendezvous in concurrent programming 
  
# Tentative Schedule

|   |   |   |
|---|---|---|
|Week 1| UNIX File System| Ch 4 |
|Week 2&3 | UNIX Env | Ch 2, 3|
|Week 4&5 | System I/O| CH 4, 5, 6|
|Week 6&7 | Processes | CH 7, 8, 9, 10|
|Week 8-10| Concurrency | Ch 11, 12|

The midterm will be given around week 5.

This schedule will vary to meet course needs.

Please note the day and time of the final. You must take the final at the time scheduled for
your class.

|CRN | Final|
|----|------|
|42481| Thursday December 12 8:00­10:00|

There will be an assignment due the last week of the term.

# Grading Policies

Grades are based on one midterm exam, one final exam, five project assignments,
and in class participation. 

|---|---|
|20%|Midterm Exam|
|20%|Final Exam|
|50%|Projects|
|10%|Participation|

Project assignments will be posted throughout the quarter. You will always have
at least a week to work on the assignment. Late work will not be accepted.
Final grades are computed as follows:

```C
char grade(float pct) {
   char g;
   assert (0 <= pct && pct <= 1.0);
   g = 'A' + (int)((1 - pct) * 10.0);
   return (g == 'E' || g > 'F' ? 'F' : g);

}
```

Pluses and minuses are assigned at my discretion. The final exam is required!

# Academic Honesty

Academic dishonesty is not tolerated at Western Washington University. Someone
commits an act of academic dishonesty when he or she participates in
representing something as the work of a student that is not in fact the work of
that student. A Western student who is caught committing such an act at Western
typically fails the course in which it occurred, and repeated such acts can lead
to dismissal from the University. For a full description of the academic honesty
policy and procedures at Western, see Appendix D in the catalog.

# Accommodations

Reasonable accommodations will be made on a case-by-case basis. Students with
documented disabilities should contact DRS at (360) 650 - 3083. Students with
unusual or unexpected challenges should talk to me.

# Flexibility

This syllabus is subject to change. Changes, if any, will be announced in
class. Students will be held responsible for all changes.

# Religious Accommodation

Western provides reasonable accommodation for students to take holidays for
reasons of faith or conscience or for organized activities conducted under the
auspices of a religious denomination, church, or religious
organization. Students seeking such accommodation must provide written notice to
their faculty within the first two weeks of the course, citing the specific
dates for which they will be absent. "Reasonable accommodation" means that
faculty will coordinate with the student on scheduling examinations or other
activities necessary for completion of the course or program and includes
rescheduling examinations or activities or offering different times for
examinations or activities. Additional information about this accommodation can
be found in SB 5166.
